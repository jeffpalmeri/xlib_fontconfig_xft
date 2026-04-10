#ifndef STRUCTS
#define STRUCTS

#ifdef DEBUG
#define de_printf(...) {\
  printf(__VA_ARGS__);\
}
#else
#define de_printf(...)
#endif

typedef struct CS {
  char buf[512];
  char priv;
  int len;
  int arg[16];
  int narg;
  char mode[2];
} CS;

typedef struct JGlyph {
  int row;
  int col;
  char c;
} JGlyph;

typedef struct Line {
  int dirty;
  int row;
  JGlyph* lineData;
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
  int offset;
  int top;
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
