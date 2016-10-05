#include <X11/Xlib.h>
#include <stdlib.h>
#include "display.h"

GraphicsContainer getNewDisplay(int x, int y){
	Display *disp = XOpenDisplay(NULL);
	int screen = DefaultScreen(disp);
	Window win = XCreateWindow(disp, DefaultRootWindow(disp),0,0,x,y,0,0,0,0,0,0);
	Atom wmDelete = XInternAtom(disp, "WM_DELETE_WINDOW", True);
	GC gc_2;
	XSetWMProtocols(disp, win, &wmDelete, 1);

	XGCValues gcvalues_2;
	gcvalues_2.function = GXcopy;
	gcvalues_2.plane_mask = AllPlanes;
	gcvalues_2.foreground = 0x00FF00;
	gcvalues_2.background = 0xFFFFFF;
	gc_2 = XCreateGC(disp, win, GCFunction|GCPlaneMask|GCForeground|GCBackground, &gcvalues_2);

	  XEvent evt;
	long eventMask = StructureNotifyMask;
	eventMask |= ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask;
	XSelectInput(disp, win, eventMask);

	XMapWindow(disp, win);

	// wait until window appears
	do { XNextEvent(disp,&evt); } while (evt.type != MapNotify);

	//while (true){
	//	if(XPending(disp)){
	//		return 0;
	//	}
	//}
	GraphicsContainer gc;
	gc.display = disp;
	gc.window = win;
	gc.gc = gc_2;
	return gc;
	




}
