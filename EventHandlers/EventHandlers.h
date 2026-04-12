#ifndef EVENTHANDLERS
#define EVENTHANDLERS

#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

void handleKeyPress(XEvent *e);

#endif
