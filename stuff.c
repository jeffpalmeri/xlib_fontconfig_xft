#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <X11/extensions/Xrender.h>
#include "stuff.h"
#include <X11/extensions/Xrender.h>

extern Display *display;
extern XftFont *font;
extern Term term;
extern CS cs;
extern XEvent evt;
extern int masterFd;
extern XftDraw *draw;
extern XftColor xft_font_color;
extern XftColor xft_bg_color;

void printCS() {
  printf("CS {\n");
  printf("  buf: %s\n", cs.buf);
  printf("  priv: %d\n", cs.priv);
  printf("  len: %d\n", cs.len);
  printf("  narg: %d\n", cs.narg);
  printf("  mode: %s\n", cs.mode);
  printf("  args: {\n");
  for (int i = 0; i < cs.narg; i++) {
    printf("    [%d]: %d\n", i, cs.arg[i]);
  }
  printf("  }\n");
  printf("}\n");
}

void drawCursor(XftFont *font, XftColor *color, XftDraw *draw) {
  XRectangle rab;
  rab.x = 0;
  rab.y = 0;
  rab.height = font->height;
  rab.width = font->max_advance_width;
  XftDrawRect(draw, color, term.cursor_x, term.cursor_y - font->ascent, rab.width, rab.height);
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
        parse_csi();
        printf("PRINTING CS!!\n");
        printCS();
        handle_csi();
      }
    }
  }
  // cs.buf[cs.len] = '\0';
  printf("DONEZOOOOOOO\n");
}

int csi_ending_char(char b) { return b > 64 && b < 126; }

void parse_csi() {
  char sep = ';';
  cs.buf[cs.len] = '\0';
  char *p = cs.buf, *np;
  if (p[0] == '?') {
    cs.priv = '?';
    p++;
  }
  long v;
  // buf = something like 2004h
  //                          ^ <-- where strtol should stop
  while (p < cs.buf + cs.len) {
    /*
     * - buf could look something like this: "?2004h" or "0;4;5m"
     * - p starts pointing to the beginning of the buf.
     * - np starts as pointing to nothing.
     * - strtol will parse the first number and then np will point
     *   to where it stopped, which should be either ";" or the
     *   final character
     * - If p == np after strtol that means p was not pointing to
     *   a valid number to begin with so nothing happened.
     * - Otherwise, we did parse an arg and can save that in
     *   cs.arg.
     * - Then move p to be at the same place as np.
     * - If there are multiple args, then p will point to ';'
     *   and then increment p and loop continues.
     * - Otherwise we've reached the end of the buf and break
     * - Finally, set cs.mode
     */
    np = NULL;

    v = strtol(p, &np, 10);
    if (np == p) {
      // strtol didn't move
      v = 0;
    }

    cs.arg[cs.narg++] = v;

    p = np;
    if (*p != sep)
      break;
    p++;
  }

  cs.mode[0] = *p;
  cs.mode[1] = '\0'; // Can there be a second? st has a check for it. I can add
                     // that later if needed
}

void handle_csi() {
  switch (cs.mode[0]) {
  case 'h':
    // turn on bracketed paste mode (will implement later)
    break;
  case 'l':
    // turn off bracketed paste mode (will implement later)
    break;
  case 'm':
    break;
  }
  memset(&cs, 0, sizeof(cs));
}

void write_char(const char *p) {
  XKeyEvent *xke = &evt.xkey;
  KeySym keysym = NoSymbol;
  char buf[64];
  int len;

    buf[0] = *p;
    buf[1] = '\0';
    printf("What's the p pointer: %c\n", *p);
    printf("What's in the buf at buf[0]: %c\n", buf[0]);
    unsigned int codepoint = (unsigned char)buf[0];
    printf("Ok getting serious, the letter typed is %s\n", buf);

    FT_UInt glyph = XftCharIndex(display, font, codepoint);
    printf("XftCharIndex() seems to be called successfully %u\n", glyph);
    // XftColor xft_font_color;
    // XRenderColor xr = {0x0000, 0x0000 , 0x0000, 0xffff};
    // XftColorAllocValue(display, visual, colormap, &xr, &xft_font_color);
    // printf("xft color allocated\n");

    int cell_width = font->max_advance_width;
    // int cell_height = font->ascent + font->descent;
    int cell_height = font->height;

    XRectangle r;
    r.x = 0;
    r.y = 0;
    r.height = cell_height;
    r.width = cell_width;

    int x = term.cursor_x;
    int y = term.cursor_y;

    // printf("****** write call, written is: %zd\n", written);
    // printf("And what the heck did I actually write?: %s\n", buf);

    // Commenting out the actual drawing of text for now
    XftDrawRect(draw, &xft_bg_color, x, y - font->ascent, cell_width, cell_height); // width and height? 
    XftDrawSetClipRectangles(draw, x, y - font->ascent, &r, 1); 
    XftGlyphFontSpec spec; 
    spec.font = font; 
    spec.glyph = glyph; 
    spec.x = x; 
    spec.y = y;

    XftDrawGlyphFontSpec(draw, &xft_font_color, &spec, 1);

    XftDrawSetClip(draw, 0);

    term.cursor_x += font->max_advance_width;

    // Draw the new cursor position, since last char has been drawn, and x
    // position updated
    drawCursor(font, &xft_font_color, draw);
  // }
}
