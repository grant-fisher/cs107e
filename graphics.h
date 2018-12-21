
#ifndef GRAPHICS_H
#define GRAPHICS_H

/* 'boundsCheck' is a helper function that checks to see if the (x,y)
 * coordinates passed in do not exceed the physical limiations of
 * our motors and canvas. It alters the x and y values by dereferencing
 * their pointers that we can safely draw to those coordinates.
 */
void boundsCheck(int *x, int *y);

/* 'moveTo' takes in a new X position and a new Y position and moves the pen 
 * to that location.  This will be called when drawing as well as when we are
 * just moving the pen to a new location on the page
 */
void moveTo(int newX, int newY, unsigned int speed);

/* 'drawLine' draws a line from a (startX, startY) coordinate to a (endX, endY)
 * coordinate.  It first checks to see if the pen is already at (startX, startY),
 * and moves the pen accordingly so it can start from that point. Then it calls the
 'move' function to move the pen in the direction of (endX, endY)
 */
void drawLine(int startX, int startY, int endX, int endY, unsigned int speed);

/* My own factorial function.  Needed to compute sin/cos
 */
double myFactorial(double x);

/* My own power function.  Needed to compute sin/cos
 */
double myPower(double x, int power);

/* My own sin function.  Needed to compute the trig relations
 * when working with drawShape()
 */
double mySin(double x);

/* My own sin function.  Needed to compute the trig relations
 * when working with drawShape()
 */
double myCos(double x); 

/* 'drawShape' takes in an (x, y) coordinate that determines the upper left 
 * corner of the rectangle that bounds the shape and the height and 
 * width of said recatnagle.  It then inscribes a shape with the 
 * determined amount of sides
 */
void drawShape(int x, int y, int sides, int height, int width, unsigned int speed);

#endif