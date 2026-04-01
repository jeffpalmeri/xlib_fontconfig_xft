#ifndef STUFF
#define STUFF

#include <X11/Xft/Xft.h>
#include "structs.h"

#define MARGIN_LEFT 50
#define MARGIN_TOP 100

void vtParse(char b);
void vtParse2(const char *p, int size);
void write_char(const char *p);
void write_char2(Line *line);

void drawCursor(XftFont *font, XftColor *color, XftDraw *draw);

XY coord_TermToWin(int x, int y);

#endif
