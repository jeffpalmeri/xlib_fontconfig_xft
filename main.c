#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include <fontconfig/fontconfig.h>

int height = 1200;
int width = 1600;

void die(const char *errString, ...) {
  va_list argPtr;

  va_start(argPtr, errString);
  vfprintf(stderr, errString, argPtr);
  va_end(argPtr);

  exit(1);
}

char* font1 = "Hack Nerd Font:pixelsize=28:antialias=true:autohint=true";

FcPattern* loadFont()
{
  if(!FcInit()) {
    die("FcInit failed\n");
  }

  FcPattern* pattern = FcNameParse((const FcChar8 *)font1);
  if(!pattern) {
    FcFini();
    die("Could not parse font name\n");
  }

  FcChar8* family;
  FcChar8* file;
  double size;
  double pixelsize;

  if(FcPatternGetString(pattern, FC_FAMILY, 0, &family) == FcResultMatch) {
    printf("Font family: %s\n", family);
  }

  if(FcPatternGetString(pattern, FC_FILE, 0, &file) == FcResultMatch) {
    printf("Font file: %s\n", file);
  } else {
    printf("No file?\n");
  }

  if(FcPatternGetDouble(pattern, FC_PIXEL_SIZE, 0, &pixelsize) == FcResultMatch) {
    printf("Pixel size: %.1f\n", pixelsize);
  } else {
    printf("No size?\n");
  }

  FcPatternAddDouble(pattern, FC_PIXEL_SIZE, 12);
  FcPatternAddDouble(pattern, FC_SIZE, 12);

  if(FcPatternGetDouble(pattern, FC_SIZE, 0, &size) == FcResultMatch) {
    printf("Font size: %.1f\n", size);
  } else {
    printf("No size?\n");
  }

  printf("*** Doing substitutes and matching now ***\n");

  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);
  FcResult result;
  FcPattern *match = FcFontMatch(NULL, pattern, &result);

  if(result != FcResultMatch) {
    fprintf(stderr, "Font match failed: %d\n", result);
  }

  printf("Font match seems to have succeeded\n");
  printf("Trying file again now\n");

  if(FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch) {
    printf("Font file: %s\n", file);
  } else {
    fprintf(stderr, "No file, but should have one\n");
  }

  printf("*** Moving on to Xft now ***\n");

  return match;
}

XftFont* openXft(Display *display, FcPattern *match)
{
  XftFont *font = XftFontOpenPattern(display, match);
  if(!font) die("XftFontOpenPattern failed\n");

  printf("Xft font opened successfully\n");

  printf("Loaded font: ascent=%d descent=%d max_advance=%d\n", 
      font->ascent, font->descent, font->max_advance_width);

  return font;
}

int main(int argc, char** argv)
{
  Display *display = XOpenDisplay(NULL);
  printf("Dispay opened\n");

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
  attrs.event_mask = ExposureMask | KeyPressMask;

  Window window = XCreateWindow(
      display,
      parent,
      0,
      0,
      width,
      height,
      0,
      XDefaultDepth(display, screen),
      InputOutput,
      visual,
      CWBackPixel | CWColormap | CWEventMask,
      &attrs
  );

  if(!window) die("XCreateWindow failed\n");

  printf("Complete, window id is %lu\n", window);

  FcPattern *match = loadFont();
  XftFont *font = openXft(display, match);

  printf("\nStarting rendering next\n");
  
  XMapWindow(display, window);
  GC gc = XCreateGC(display, window, 0, NULL);
  XSetForeground(display, gc, BlackPixel(display, screen));
  XFillRectangle(display, window, gc, 0, 0, 800, 800);
  XFreeGC(display, gc);

  XMapWindow(display, window);
  XFlush(display);

  XEvent evt;

  while(1) {
    XNextEvent(display, &evt);
    printf("Event type is %d\n", evt.type);

    if(evt.type == KeyPress) {
      XKeyEvent *xke = &evt.xkey;
      KeySym keysym = NoSymbol;
      char buf[64];
      int len;

      len = XLookupString(xke, buf, sizeof buf, &keysym, NULL);
      printf("KeyPress len is: %d\n", len);
      printf("KeyPress buf is: %s\n", buf);
      printf("KeyPress buf in hex: ");
      for(int i = 0; i < len; i++) {
        printf("0x%02x ", (unsigned char)buf[i]);
      }
      printf("\n");
    }
  }

  return 0;
}

