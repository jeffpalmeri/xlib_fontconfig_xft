#include <X11/Xft/Xft.h>

enum terminal_mode {
  csi
};

enum escape_state {
  ESC_START = 1 << 0, // 1
  ESC_CSI   = 1 << 1, // 2;
};

typedef struct {
  int row;
  int col;
  int cursor_x;
  int cursor_y;
  int mode;
  int esc;
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

void drawCursor(XftFont *font, XftColor *color, XftDraw *draw);
