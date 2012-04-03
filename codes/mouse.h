#include <X11/Xlib.h>

#ifndef MOUSE_H
#define MOUSE_H

// Simulate mouse click
void click (Display *display, int button);

// Releasing click
void clickrelease (Display *display, int button);

// Move mouse pointer
void move (Display *display, int x, int y);

// Get mouse coordinates
void coords (Display *display, int *x, int *y);

// Get pixel color at coordinates x,y
//void pixel_color (Display *display, int x, int y, XColor *color);

#endif
