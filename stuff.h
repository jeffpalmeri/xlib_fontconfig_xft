#include <X11/Xft/Xft.h>

#define MARGIN_LEFT 50
#define MARGIN_TOP 100

enum terminal_mode {
  csi
};

enum escape_state {
  ESC_START = 1 << 0, // 1
  ESC_CSI   = 1 << 1, // 2;
};


typedef struct Line {
  int row;
  int col;
  int dirty;
  char c;
  // glyph
} Line;

typedef struct Term {
  int rows;
  int cols;
  int cursor_x;
  int cursor_y;
  int mode;
  int esc;
  Line **lines;
} Term;

typedef struct CS {
  char buf[512];
  char priv;
  int len;
  int arg[16];
  int narg;
  char mode[2];
} CS;

void printCS();
void vtParse(char b);
void vtParse2(const char *p, int size);
void vtParse3(const char *p, int size, void (*wc)(const char*));
int csi_ending_char(char b);
void parse_csi();
void handle_csi();
void write_char(const char *p);
void write_char2(Line *line);

void drawCursor(XftFont *font, XftColor *color, XftDraw *draw);

typedef struct XY {
  int x;
  int y;
} XY;
XY coord_TermToWin(int x, int y);
