#ifndef STUFF
#define STUFF

#include <X11/Xft/Xft.h>
#include "structs.h"

#define MARGIN_LEFT 50
#define MARGIN_TOP 100

void write_char(JGlyph *glyph);

void drawCursor(XftFont *font, XftColor *color, XftDraw *draw);
void eraseCursor(XftFont *font, XftColor *color, XftDraw *draw);

XY coord_TermToWin(int x, int y);

void renderTerm();
#endif
