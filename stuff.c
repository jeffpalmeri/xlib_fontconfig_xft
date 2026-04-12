#include "stuff.h"
#include "moreStuff.h"
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <unistd.h>

#define ASCENT_MULT 1.3

extern Display *display;
extern XftFont *font;
extern Term term;
extern Window window;
extern CS cs;
extern XEvent evt;
extern int masterFd;
extern XftDraw *draw;
extern XftColor xft_font_color;
extern XftColor xft_bg_color;

void printTermState(Term *term) {
  de_printf("========\n");
  de_printf("cursor_x: %i\n", term->cursor_x);
  de_printf("cursor_y: %i\n", term->cursor_y);
  de_printf("offset: %i\n", term->offset);
  de_printf("top: %i\n", calc_top(term));
  for (int i = 0; i < term->rows; i++) {
    de_printf("[");
    for (int j = 0; j < term->cols; j++) {
      if (term->lines[i]->lineData[j].c == '\0')
        break;
      de_printf("%c, ", term->lines[i]->lineData[j].c);
    }
    de_printf("]");
    if (term->lines[i]->dirty == 1) {
      de_printf(" [DIRTY]");
    } else {
      de_printf(" [CLEAN]");
    }
    if (i == (term->cursor_x + 0) % term->rows) {
      de_printf(" <----- current");
    }
    de_printf("\n");
  }
  de_printf("========\n");
}

void printTermState2(Term *term) {
  de_printf("========\n");
  de_printf("cursor_x: %i\n", term->cursor_x);
  de_printf("cursor_y: %i\n", term->cursor_y);
  de_printf("offset: %i\n", term->offset);
  for (int i = 0; i < term->rows; i++) {
    de_printf("[");
    for (int j = 0; j < term->cols; j++) {
      if (term->lines[i]->lineData[j].c == '\0')
        break;
    }
    de_printf("]");
    if (i == (term->cursor_x + term->offset) % term->rows) {
      de_printf(" <----- current");
    }
    de_printf("\n");
  }
  de_printf("========\n");
}

XY coord_TermToWin(int x, int y) {
  int true_x = x;
  if (term.offset >= term.rows) {
    int top_row = calc_top(&term);
    true_x = ((x - top_row) + term.rows) % term.rows;
  }
  int xp = MARGIN_LEFT + (y)*font->max_advance_width;
  int yp = MARGIN_TOP + ((true_x) * (font->ascent * ASCENT_MULT));

  return (XY){xp, yp};
}

void drawCursor(XftFont *font, XftColor *color, XftDraw *draw) {
  term.old_cursor_x = term.cursor_x;
  term.old_cursor_y = term.cursor_y;
  XRectangle rab;
  rab.x = 0;
  rab.y = 0;
  rab.height = font->height;
  rab.width = font->max_advance_width;
  XY c = coord_TermToWin(term.cursor_x, term.cursor_y);
  XftDrawRect(draw, color, c.x, c.y - font->ascent, rab.width, rab.height);
}

void eraseCursor(XftFont *font, XftColor *color, XftDraw *draw) {
  XRectangle rab;
  rab.x = 0;
  rab.y = 0;
  rab.height = font->height;
  rab.width = font->max_advance_width;
  XY c = coord_TermToWin(term.old_cursor_x, term.old_cursor_y);
  XftDrawRect(draw, color, c.x, c.y - font->ascent, rab.width, rab.height);
}

void write_char(JGlyph *gly) {
  if (gly->c == '\0')
    return;
  char *p = &gly->c;
  XKeyEvent *xke = &evt.xkey;
  KeySym keysym = NoSymbol;
  char buf[64];
  int len;

  buf[0] = *p;
  buf[1] = '\0';
  unsigned int codepoint = (unsigned char)buf[0];

  FT_UInt glyph = XftCharIndex(display, font, codepoint);

  de_printf("What's the p pointer: %d\n", *p);
  de_printf("What's in the buf at buf[0]: %c\n", buf[0]);
  de_printf("Ok getting serious, the letter typed is %s\n", buf);
  de_printf("XftCharIndex() seems to be called successfully %u\n", glyph);

  int cell_width = font->max_advance_width;
  int cell_height = font->height;

  XRectangle r;
  r.x = 0;
  r.y = 0;
  r.height = cell_height;
  r.width = cell_width;

  int x_offset = gly->row - term.offset;
  if (x_offset < 0) {
    x_offset += term.rows;
  }
  XY c = coord_TermToWin(gly->row, gly->col);

  XftDrawRect(draw, &xft_bg_color, c.x, c.y - font->ascent, cell_width,
              cell_height); // width and height?
  XftDrawSetClipRectangles(draw, c.x, c.y - font->ascent, &r, 1);
  XftGlyphFontSpec spec;
  spec.font = font;
  spec.glyph = glyph;
  spec.x = c.x;
  spec.y = c.y;

  XftDrawGlyphFontSpec(draw, &xft_font_color, &spec, 1);

  XftDrawSetClip(draw, 0);

  // XFlush(display);
  // printTermState(&term);
}

void renderTerm() {
  eraseCursor(font, &xft_bg_color, draw);
  for (int x = 0; x < term.rows; x++) {
    if (term.lines[x]->dirty == 1) {
      // The idea here is the XClearArea is not needed if writing
      // on the same line as before. I just tried this and it seems
      // to work, but I should think more about if it's solid logic.
      if (term.old_cursor_x != term.cursor_x) {
        XY c = coord_TermToWin(x, 0);
        XClearArea(display, window,
                   c.x,                // x
                   c.y - font->ascent, // y
                   2000,               // width
                   font->height,       // height
                   0);
      }
      int y = 0;
      while (y < term.cols && term.lines[x]->lineData[y].c != '\0') {
        write_char(&term.lines[x]->lineData[y]);
        y++;
      }
      term.lines[x]->dirty = 0;
    }
  }
  drawCursor(font, &xft_font_color, draw);
}
