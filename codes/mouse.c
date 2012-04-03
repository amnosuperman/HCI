#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include "mouse.h"

// Left click : 1
// Right click : 3


// Simulate mouse click
void click (Display *display, int button)
{
  	// Create and setting up the event
	XEvent event;
	memset (&event, 0, sizeof (event));
	event.xbutton.button = button;
	event.xbutton.same_screen = True;
	event.xbutton.subwindow = DefaultRootWindow (display);
        while(event.xbutton.subwindow)
        {
      		event.xbutton.window = event.xbutton.subwindow;
      		XQueryPointer (display, event.xbutton.window,&event.xbutton.root, &event.xbutton.subwindow,&event.xbutton.x_root, &event.xbutton.y_root,
		&event.xbutton.x, &event.xbutton.y,&event.xbutton.state);
    	}
  	// Press
	event.type = ButtonPress;
  	if (XSendEvent (display, PointerWindow, True, ButtonPressMask, &event) == 0)
    		fprintf (stderr, "Error to send the event!\n");
  	XFlush (display);
  	usleep (1);
}

// Release click
void clickrelease (Display *display, int button)
{
  	// Create and setting up the event
	XEvent event;
	memset (&event, 0, sizeof (event));
	event.xbutton.button = button;
	event.xbutton.same_screen = True;
	event.xbutton.subwindow = DefaultRootWindow (display);
        while(event.xbutton.subwindow)
        {
      		event.xbutton.window = event.xbutton.subwindow;
      		XQueryPointer (display, event.xbutton.window,&event.xbutton.root, &event.xbutton.subwindow,&event.xbutton.x_root, &event.xbutton.y_root,
		&event.xbutton.x, &event.xbutton.y,&event.xbutton.state);
    	}
  	// Release
  	event.type = ButtonRelease;
  	if (XSendEvent (display, PointerWindow, True, ButtonReleaseMask, &event) == 0)
    		fprintf (stderr, "Error to send the event!\n");
  	XFlush (display);
  	usleep (1);
}

// Move mouse pointer
void move (Display *display, int x, int y)
{
  	XWarpPointer (display, None, None, 0,0,0,0, x, y);
  	XFlush (display);
  	usleep (1);
}

// Get mouse coordinates
void coords (Display *display, int *x, int *y)
{
  	XEvent event;
  	XQueryPointer (display, DefaultRootWindow (display),&event.xbutton.root, &event.xbutton.window,&event.xbutton.x_root, &event.xbutton.y_root,
        &event.xbutton.x, &event.xbutton.y,&event.xbutton.state);
	*x = event.xbutton.x;
	*y = event.xbutton.y;
}

// Get pixel color at coordinates x,y
/*void pixel_color (Display *display, int x, int y, XColor *color)
{
 	XImage *image;
	image = XGetImage (display, DefaultRootWindow (display), x, y, 1, 1, AllPlanes, XYPixmap);
	color->pixel = XGetPixel (image, 0, 0);
	XFree (image);
	XQueryColor (display, DefaultColormap(display, DefaultScreen (display)), color);
}*/

