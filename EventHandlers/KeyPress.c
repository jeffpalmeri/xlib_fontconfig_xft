#include "EventHandlers.h"

extern int masterFd;

void handleKeyPress(XEvent *e) {
  if (e->type == KeyPress) {
    XKeyEvent *xke = &e->xkey;
    KeySym keysym = NoSymbol;
    char buf[64];
    int len;

    len = XLookupString(xke, buf, sizeof buf, &keysym, NULL);
    // de_printf("==========\n");
    // de_printf("KeyPress len is: %d\n", len);
    // de_printf("KeyPress buf is: %s\n", buf);
    // de_printf("KeyPress buf in hex: ");
    // for (int i = 0; i < len; i++) {
    //   de_printf("0x%02x ", (unsigned char)buf[i]);
    // }
    // de_printf("\n");
    // de_printf("KeySym is: %lu\n", keysym);
    // de_printf("KeySym to string is: %s\n", XKeysymToString(keysym));
    // de_printf("==========\n\n");
    //
    // if (xke->state & Mod1Mask) {
    //   de_printf("Typed with ALT held down and len is %d\n", len);
    // }

    /*
    // if(keysym == 65293) { // return
    if (keysym == XK_Return) { // return
      // Delete the cursor from the end of the line
      drawCursor(font, &xft_bg_color, draw);

      // XK_Return;
      // term.cursor_y += 50;
      term.cursor_y += font->height * 1.2;
      term.cursor_x = 50;

      // Draw cursor now at the start of the new line
      drawCursor(font, &xft_font_color, draw);

      ssize_t written = write(masterFd, "\n", 1);

      continue;
    }

    if (keysym == 65288) { // backspace
                           //
      de_printf("Backspace!\n");
      // Delete the previous cursor
      drawCursor(font, &xft_bg_color, draw);

      int cell_width = font->max_advance_width;
      int cell_height = font->height;
      XRectangle r;
      r.x = 0;
      r.y = 0;
      r.height = cell_height;
      r.width = cell_width;

      term.cursor_x -= font->max_advance_width;

      int x = term.cursor_x;
      int y = term.cursor_y;

      XftDrawRect(draw, &xft_bg_color, x, y - font->ascent, cell_width,
                  cell_height); // width and height?
      XftDrawSetClipRectangles(draw, x, y - font->ascent, &r, 1);
      XftDrawSetClip(draw, 0);

      // Draw new cursor after the backspace
      drawCursor(font, &xft_font_color, draw);

      continue;
    }
    */
    if (len > 0) {
      // Maybe using XKeysymToString is the more correct way than doing
      // this?
      // buf[len] = '\0';
      // unsigned int codepoint = (unsigned char)buf[0];
      // de_printf("Ok getting serious, the letter typed is %s\n", buf);
      //
      // FT_UInt glyph = XftCharIndex(display, font, codepoint);
      // de_printf("XftCharIndex() seems to be called successfully %u\n",
      // glyph); int cell_width = font->max_advance_width; int cell_height
      // = font->height;
      //
      // XRectangle r;
      // r.x = 0;
      // r.y = 0;
      // r.height = cell_height;
      // r.width = cell_width;
      //
      // int x = term.cursor_x;
      // int y = term.cursor_y;

      ssize_t written = write(masterFd, buf, len);
    }
  }
}
