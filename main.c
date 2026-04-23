#include <X11/extensions/Xrender.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>

#include <fontconfig/fontconfig.h>

#include "pty.h"
#include "ptyFork.h"

#include "moreStuff.h"
#include "structs.h"
#include "stuff.h"
#include "EventHandlers/EventHandlers.h"

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MAX_SNAME 1000

int height = 1200;
int width = 1600;

void die(const char *errString, ...) {
  va_list argPtr;

  va_start(argPtr, errString);
  vfprintf(stderr, errString, argPtr);
  va_end(argPtr);

  exit(1);
}

Display *display;
XftFont *font;
XftColor xft_font_color;
XftColor xft_bg_color;
XftDraw *draw;
Term term;
Window window;
CS cs;
XEvent evt;
int masterFd;

void drawGlyph() {}

static void (*handler[LASTEvent])(XEvent *) = {
    [KeyPress] = handleKeyPress,
};

char *font1 = "Hack Nerd Font:pixelsize=28:antialias=true:autohint=true";

FcPattern *loadFont() {
  if (!FcInit()) {
    die("FcInit failed\n");
  }

  FcPattern *pattern = FcNameParse((const FcChar8 *)font1);
  if (!pattern) {
    FcFini();
    die("Could not parse font name\n");
  }

  FcChar8 *family;
  FcChar8 *file;
  double size;
  double pixelsize;

  if (FcPatternGetString(pattern, FC_FAMILY, 0, &family) == FcResultMatch) {
    de_printf("Font family: %s\n", family);
  }

  if (FcPatternGetString(pattern, FC_FILE, 0, &file) == FcResultMatch) {
    de_printf("Font file: %s\n", file);
  } else {
    de_printf("No file?\n");
  }

  if (FcPatternGetDouble(pattern, FC_PIXEL_SIZE, 0, &pixelsize) ==
      FcResultMatch) {
    de_printf("Pixel size: %.1f\n", pixelsize);
  } else {
    de_printf("No size?\n");
  }

  FcPatternAddDouble(pattern, FC_PIXEL_SIZE, 12);
  FcPatternAddDouble(pattern, FC_SIZE, 12);

  if (FcPatternGetDouble(pattern, FC_SIZE, 0, &size) == FcResultMatch) {
    de_printf("Font size: %.1f\n", size);
  } else {
    de_printf("No size?\n");
  }

  de_printf("*** Doing substitutes and matching now ***\n");

  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);
  FcResult result;
  FcPattern *match = FcFontMatch(NULL, pattern, &result);

  if (result != FcResultMatch) {
    fprintf(stderr, "Font match failed: %d\n", result);
  }

  de_printf("Font match seems to have succeeded\n");
  de_printf("Trying file again now\n");

  if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch) {
    de_printf("Font file: %s\n", file);
  } else {
    fprintf(stderr, "No file, but should have one\n");
  }

  de_printf("*** Moving on to Xft now ***\n");

  return match;
}

XftFont *openXft(Display *display, FcPattern *match) {
  font = XftFontOpenPattern(display, match);
  if (!font)
    die("XftFontOpenPattern failed\n");

  de_printf("Xft font opened successfully\n");

  de_printf("Loaded font: ascent=%d descent=%d max_advance=%d\n", font->ascent,
            font->descent, font->max_advance_width);

  return font;
}

int main(int argc, char **argv) {
  memset(&cs, 0, sizeof(cs));
  const int maxSnLen = 1000;
  char slaveName[1000];

  struct termios ttyOrig;
  struct winsize ws;
  char *shell;
  if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1) {
    die("tcgetattr");
  }
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0) {
    die("ioctl-TIOCGWINSZ");
  }
  // int masterFd = ptyMasterOpen(slaveName, maxSnLen);
  pid_t childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
  if (childPid == 0) {
    // shell = getenv("SHELL");
    // if(shell == NULL || *shell == '\0') shell = "/bin/sh";
    shell = "/bin/sh";
    execlp(shell, shell, (char *)NULL);
    die("execlp"); // If we get here, something went wrong
  }

  de_printf("masterFd %i\n", masterFd);

  term = (Term){5, 160, 0, 0, 0, 0, 0, 0, 0, 0};
  // How big will each "line" be?
  // #rows * sizeof(Line*),
  // and each Line will be (JGlyph * #cols) + int + int
  term.lines = malloc(sizeof(Line *) * term.rows);
  for (int i = 0; i < term.rows; i++) {
    term.lines[i] = malloc(sizeof(Line));
    // term.lines[i] = malloc((sizeof(JGlyph) * term.cols) + (2*sizeof(int)));
    term.lines[i]->dirty = 1;
    term.lines[i]->row = i;
    term.lines[i]->lineData = malloc(sizeof(JGlyph) * term.cols);
    for(int j = 0; j < term.cols; j++) {
      term.lines[i]->lineData[j].row = i;
      term.lines[i]->lineData[j].col = j;
    }
  }
  // term.lines = malloc(sizeof(Line*) * term.rows);
  // for(int i = 0; i < term.rows; i++) {
  //   term.lines[i] = malloc(sizeof(Line) * term.cols);
  // }

  display = XOpenDisplay(NULL);
  de_printf("Dispay opened\n");

  int screen = XDefaultScreen(display);
  Visual *visual = XDefaultVisual(display, screen);

  Window parent = XRootWindow(display, screen);

  XColor grey;
  Colormap colormap = DefaultColormap(display, screen);
  XParseColor(display, colormap, "#808080", &grey);
  XAllocColor(display, colormap, &grey);

  XSetWindowAttributes attrs = {0};

  attrs.background_pixel = grey.pixel;
  attrs.colormap = XCreateColormap(display, parent, visual, AllocNone);
  attrs.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

  window = XCreateWindow(display, parent, 0, 0, width, height, 0,
                         XDefaultDepth(display, screen), InputOutput, visual,
                         CWBackPixel | CWColormap | CWEventMask, &attrs);

  if (!window)
    die("XCreateWindow failed\n");

  de_printf("Complete, window id is %lu\n", window);

  FcPattern *match = loadFont();
  XftFont *font = openXft(display, match);
  draw = XftDrawCreate(display, window, visual, colormap);

  XRenderColor xr = {0x0000, 0x0000, 0x0000, 0xffff};
  XftColorAllocValue(display, visual, colormap, &xr, &xft_font_color);

  XRenderColor bgxr = {grey.red, grey.green, grey.blue, 0xffff};
  XftColorAllocValue(display, visual, colormap, &bgxr, &xft_bg_color);

  de_printf("xft colors allocated\n");

  de_printf("\nStarting rendering next\n");

  XMapWindow(display, window);
  GC gc = XCreateGC(display, window, 0, NULL);
  XSetForeground(display, gc, BlackPixel(display, screen));
  XFillRectangle(display, window, gc, 0, 0, 800, 800);

  XMapWindow(display, window);
  XFlush(display);

  do {
    XNextEvent(display, &evt);
  } while (evt.type != MapNotify);
  de_printf("after the do while loop\n");

  int xfd = XConnectionNumber(display);
  char buf[256];
  while (1) {
    fd_set rfd;
    FD_ZERO(&rfd);
    FD_SET(masterFd, &rfd);
    FD_SET(xfd, &rfd);
    //   struct timespec seltv, *tv;
    // tv = timeout >= 0 ? &seltv : NULL;

    if (pselect(MAX(xfd, masterFd) + 1, &rfd, NULL, NULL, NULL, NULL) < 0) {
      if (errno == EINTR)
        continue;
      die("select failed: %s\n", strerror(errno));
    }

    if (FD_ISSET(masterFd, &rfd)) {
      ssize_t numRead = read(masterFd, buf, 256);
      de_printf("**** numRead from masterFd: %zd\n", numRead);
      de_printf("And what the heck did I actually read?: %.*s\n", numRead, buf);
      // de_printf("And what the heck did I actually read2?: 0x%02x\n", (unsigned char)buf);

      // Helpful debug for seeing shell bytes
      fprintf(stdout, "\n----------start------------\n");
      for (ssize_t i = 0; i < numRead; i++) {
        fprintf(stdout, "%02x ", (unsigned char)buf[i]);
      }
      fprintf(stdout, "\n----------end------------\n");

      vtParse(buf, numRead, &term, &cs, handle_csi);
      renderTerm();
    }

    while (XPending(display)) {
      XNextEvent(display, &evt);
      // de_printf("---------start-------------\n");
      // de_printf("Event type is %d\n", evt.type);
      if (handler[evt.type]) {
        handler[evt.type](&evt);
      }
    }
  }

  return 0;
}

// (lldb) p term.lines[0][0]
// (Line)  (row = 50, col = 100, dirty = 1, c = 's')
// (lldb) p term.lines[0][1]
// (Line)  (row = 50, col = 101, dirty = 1, c = 'h')
// (lldb) p term.lines[0][2]
// (Line)  (row = 50, col = 102, dirty = 1, c = '-')
// (lldb) p term.lines[0][3]
// (Line)  (row = 50, col = 103, dirty = 1, c = '5')
// (lldb) p term.lines[0][4]
// (Line)  (row = 50, col = 104, dirty = 1, c = '.')
// (lldb) p term.lines[0][5]
// (Line)  (row = 50, col = 105, dirty = 1, c = '3')
// (lldb) p term.lines[0][6]
// (Line)  (row = 50, col = 106, dirty = 1, c = '$')
// (lldb) p term.lines[0][7]
// (Line)  (row = 50, col = 107, dirty = 1, c = ' ')
// (lldb) p term.lines[0][8]
// (Line)  (row = 0, col = 0, dirty = 0, c = '\0')
//
// break set --file main.c --line 298 -c '(term.lines[xp][y].dirty == 1)'
// break set --file main.c --line 296 -c '(term.offset == 1)'

/*
   const Lines = [
      {
        dirty = 1,
        data: {
          row: 0,
          col: 0,
          c: "j"
        }
      },
      {
        dirty = 0,
        data: {
          row: 0,
          col: 1,
          c: "e"
        }
      }
   ]
 * */


// #define XK_BackSpace   0xff08  /* U+0008 BACKSPACE */
// #define XK_Left        0xff51  /* Move left, left arrow */
//
//
// case 'P': /* DCH -- Delete <n> char */
// 	DEFAULT(csiescseq.arg[0], 1);
// 	tdeletechar(csiescseq.arg[0]);
// 	break;
//
//
// case 'C': /* CUF -- Cursor <n> Forward */
// case 'a': /* HPR -- Cursor <n> Forward */
// 	DEFAULT(csiescseq.arg[0], 1);
// 	tmoveto(term.c.x+csiescseq.arg[0], term.c.y);
// 	break;
//
// 	abcdefg........LLLLB ----> CCCK
//             ^    
//                ^=======
// /usr/include/X11/keysymdef.h
// tomoveto
