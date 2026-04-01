#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
// #include <X11/extensions/Xrender.h>
#include "stuff.h"
#include "moreStuff.h"
#include <X11/extensions/Xrender.h>

#define ASCENT_MULT 1.3

extern Display *display;
extern XftFont *font;
extern Term term;
extern CS cs;
extern XEvent evt;
extern int masterFd;
extern XftDraw *draw;
extern XftColor xft_font_color;
extern XftColor xft_bg_color;

XY coord_TermToWin(int x, int y) {
  int xp = MARGIN_LEFT + (y)*font->max_advance_width;
  int yp = MARGIN_TOP + ((x)*(font->ascent * ASCENT_MULT));
  return (XY){xp, yp};
}

void drawCursor(XftFont *font, XftColor *color, XftDraw *draw) {
  XRectangle rab;
  rab.x = 0;
  rab.y = 0;
  rab.height = font->height;
  rab.width = font->max_advance_width;
  XY c = coord_TermToWin(term.cursor_x, term.cursor_y);
  XftDrawRect(draw, color, 
      c.x, c.y-font->ascent,
      rab.width, rab.height);
}

void vtParse(char b) {
  // if esc = 0 byte is espcape esc state should change to esc start or
  // something
  if (term.esc == 0) {
    if (b == 0x1b) {
      term.esc |= ESC_START;
      return;
    }
  }
  // if(term.esc == 1) {
  if (term.esc & ESC_START) {
    if (b == 0x5b) {
      term.esc |= ESC_CSI;
      return;
    }
  }
  if (term.esc && ESC_CSI) {
    cs.buf[cs.len++] = b;
  }
}

void vtParse2(const char *p, int size) {
  for (int i = 0; i < size; i++) {
    char b = p[i];
    if (term.esc == 0) {
      if (b == 0x1b) {
        term.esc |= ESC_START;
        continue;
      }

      // write normal char here?
      write_char(p+i);
      continue;
    }
    if (term.esc & ESC_START) {
      if (b == 0x5b) {
        term.esc |= ESC_CSI;
        continue;
      }
    }
    if (term.esc && ESC_CSI) {
      cs.buf[cs.len++] = b;
      if (csi_ending_char(b)) { // 64-126
        term.esc = 0;
        parse_csi(&cs);
        printf("PRINTING CS!!\n");
        printCS(&cs);
        handle_csi(&cs);
      }
    }
  }
  // cs.buf[cs.len] = '\0';
  printf("DONEZOOOOOOO\n");
}

void write_char(const char *p) {
  printf("THIS SHOULDNT EXISTRRRRRRRRRR");
  return;
  // XKeyEvent *xke = &evt.xkey;
  // KeySym keysym = NoSymbol;
  // char buf[64];
  // int len;
  //
  //   buf[0] = *p;
  //   buf[1] = '\0';
  //   printf("What's the p pointer: %c\n", *p);
  //   printf("What's in the buf at buf[0]: %c\n", buf[0]);
  //   unsigned int codepoint = (unsigned char)buf[0];
  //   printf("Ok getting serious, the letter typed is %s\n", buf);
  //
  //   FT_UInt glyph = XftCharIndex(display, font, codepoint);
  //   printf("XftCharIndex() seems to be called successfully %u\n", glyph);
  //   // XftColor xft_font_color;
  //   // XRenderColor xr = {0x0000, 0x0000 , 0x0000, 0xffff};
  //   // XftColorAllocValue(display, visual, colormap, &xr, &xft_font_color);
  //   // printf("xft color allocated\n");
  //
  //   int cell_width = font->max_advance_width;
  //   // int cell_height = font->ascent + font->descent;
  //   int cell_height = font->height;
  //
  //   XRectangle r;
  //   r.x = 0;
  //   r.y = 0;
  //   r.height = cell_height;
  //   r.width = cell_width;
  //
  //   int x = term.cursor_x*font->max_advance_width;
  //   int y = term.cursor_y*font->ascent;
  //
  //   // printf("****** write call, written is: %zd\n", written);
  //   // printf("And what the heck did I actually write?: %s\n", buf);
  //
  //   // Commenting out the actual drawing of text for now
  //   XftDrawRect(draw, &xft_bg_color, x, y - font->ascent, cell_width, cell_height); // width and height? 
  //   XftDrawSetClipRectangles(draw, x, y - font->ascent, &r, 1); 
  //   XftGlyphFontSpec spec; 
  //   spec.font = font; 
  //   spec.glyph = glyph; 
  //   spec.x = x; 
  //   spec.y = y;
  //
  //   XftDrawGlyphFontSpec(draw, &xft_font_color, &spec, 1);
  //
  //   XftDrawSetClip(draw, 0);
  //
  //   // term.cursor_x += font->max_advance_width;
  //
  //   // Draw the new cursor position, since last char has been drawn, and x
  //   // position updated
  //   drawCursor(font, &xft_font_color, draw);
  // }
  //
}

void write_char2(Line *line) {
  char *p = &line->c;
  XKeyEvent *xke = &evt.xkey;
  KeySym keysym = NoSymbol;
  char buf[64];
  int len;

    buf[0] = *p;
    buf[1] = '\0';
    printf("What's the p pointer: %d\n", *p);
    printf("What's in the buf at buf[0]: %c\n", buf[0]);
    unsigned int codepoint = (unsigned char)buf[0];
    printf("Ok getting serious, the letter typed is %s\n", buf);

    FT_UInt glyph = XftCharIndex(display, font, codepoint);
    printf("XftCharIndex() seems to be called successfully %u\n", glyph);

    int cell_width = font->max_advance_width;
    int cell_height = font->height;

    XRectangle r;
    r.x = 0;
    r.y = 0;
    r.height = cell_height;
    r.width = cell_width;

    XY c = coord_TermToWin(line->row, line->col);

    XftDrawRect(draw, &xft_bg_color, c.x, c.y - font->ascent, cell_width, cell_height); // width and height? 
    XftDrawSetClipRectangles(draw, c.x, c.y - font->ascent, &r, 1); 
    XftGlyphFontSpec spec; 
    spec.font = font; 
    spec.glyph = glyph; 
    spec.x = c.x; 
    spec.y = c.y;


    XftDrawGlyphFontSpec(draw, &xft_font_color, &spec, 1);

    XftDrawSetClip(draw, 0);


    // term.cursor_x += font->max_advance_width;

    // Draw the new cursor position, since last char has been drawn, and x
    // position updated
    drawCursor(font, &xft_font_color, draw);
}
