#ifndef STRUCTS
#define STRUCTS

typedef struct CS {
  char buf[512];
  char priv;
  int len;
  int arg[16];
  int narg;
  char mode[2];
} CS;

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
  int old_cursor_x;
  int old_cursor_y;
  Line **lines;
} Term;

typedef struct XY {
  int x;
  int y;
} XY;

enum terminal_mode {
  csi
};

enum escape_state {
  ESC_START = 1 << 0, // 1
  ESC_CSI   = 1 << 1, // 2;
};

#endif
