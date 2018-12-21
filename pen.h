#ifndef PEN_H
#define PEN_H

/* Moves pen up, changing width from 2500 to 5000. If the pen is already
 * up, it does nothing.
 */
void penUp(void);

/* Moves pen down. If the pen is already down, it does nothing.
 */ 
void penDown(void);

/* Initializes GPIO pin 18 and sets the clock, range, and width for the servo motor.
 */ 
void pen_init(void);

#endif