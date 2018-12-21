#ifndef MOUSE_H
#define MOUSE_H

/*
 * Author: Omar Rizwan <osnr@stanford.edu>
 *
 * Date: April 30, 2016
 */

typedef struct {
    int x_overflow, y_overflow;
    int dx, dy;
    int left, middle, right;
} mouse_event_t;

/*
 * `mouse_init`: Required initialization for mouse
 *
 * The mouse must first be initialized before any mouse events can be read.
 * The mouse clock line should be connected to GPIO_PIN25 and data line to GPIO_PIN26.
 */
void mouse_init(void);

/*
 * `mouse_read_event`
 *
 * The function reads (blocking) the next event from the mouse.
 * Returns a `mouse_event_t` struct that represents the mouse event.  The
 * struct includes the relative change in x and y (dx, dy fields) (will be
 * in range +- 255), the x_overflow/y_overflow fields are 1 if true value
 * of dx/dy exceeded +/- 255.  The left/middle/right fields give the state of
 * the mouse buttons. Each field is 1 if the corresponding mouse button 
 * is down, 0 otherwise.
 */
mouse_event_t mouse_read_event(void);

/*
 * `mouse_read_scancode`
 *
 * The function reads (blocking) the next PS/2 scancode from the mouse.
 * Operates very similar to keyboard_read_scancode.
 */
unsigned char mouse_read_scancode(void);

void notFirstInit(void);

#endif
