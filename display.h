#ifndef DISPLAY_H
#define DISPLAY_H

#include <X11/Xlib.h>

struct GraphicsContainer{
	Display *display;
	Window window;
	GC gc;
};

GraphicsContainer getNewDisplay(int height, int width);
#endif
