// A graphics library for our CNC machine.  Gives the user the ability to 
// draw lines as well as draw polygons.  Because we wanted to use code
// that was only written by us, we created our own sin and cos functions
// using a Taylor Series approximation.  These called our own power
// and factorial functions.
 
#include "pen.h"
#include "axis.h"
#include "printf.h"

#define WIDTH 450 // Physical limit of our stepper motor
#define HEIGHT 450 // Physical limit of our stepper motor
#define MYPICONST 3.141592653 // Constant for PI

int Xcoor = 0; // Globally keeps track of the x coordinate of the pen
int Ycoor = 0; // Globally keeps track of the y coordinate of the pen
const int THRESHOLD = 20; // Threshold of how many orders deep we go in Taylor approx

// Checks to see if the coordinates passed in exceed the physical limits 
// of the stepper motors.  It truncates any values that are beyond physical
// limits.
 
void boundsCheck(int *x, int *y) {
	if (*x > WIDTH) {
		*x = WIDTH;
	}
	else if (*x < 0) {
		*x = 0;
	}
	if (*y > HEIGHT) {
		*y = HEIGHT;
	}
	else if (*y < 0) {
		*y = 0;
	}
}

// Moves the pen to a new (x, y) location with a given speed.  Keeps
// track globally of where the pen now is.
void moveTo(int newX, int newY, unsigned int speed) {
	boundsCheck(&newX, &newY);
	int dx = newX - Xcoor;
	int dy = newY - Ycoor;
	move(dx, dy, speed);
	Xcoor = newX;
	Ycoor = newY;
}

// Draws a line from (startX, startY) to (endX, endY) at a given speed
void drawLine(int startX, int startY, int endX, int endY, unsigned int speed) {
	if (startX != Xcoor || startY != Ycoor) { // If the pen isn't currently in the right spot
		penUp();
		moveTo(startX, startY, speed);
	}
	penDown();
	moveTo(endX, endY, speed);
}

// Recursive factorial function.
double myFactorial(double x) {
	if (x > 0) {
		return (x * myFactorial(x - 1));
	}
	return 1;
}

// Power function.  Raises x to the given power.
double myPower(double x, int power) {
	double nextX = 1;
	for (int i = 0; i < power; i++) {
		nextX *= x;
	}
	return nextX;
}

// Sin function that uses a Taylor series to compute its value.
double mySin(double x) {
	double returnX = x;
    for (int i = 3; i <= THRESHOLD; i += 4) {
        returnX -= (myPower(x, i) / myFactorial(i));
    }
    for (int j = 5; j <= THRESHOLD; j += 4) {
        returnX += (myPower(x, j) / myFactorial(j));
    }
	return returnX;
}

// Cos function that uses a Taylor series to compute its value.
double myCos(double x) {
	double returnX = 1;
    for (int i = 2; i <= THRESHOLD; i += 4) {
        returnX -= (myPower(x, i) / myFactorial(i));
    }
    for (int j = 4; j <= THRESHOLD; j += 4) {
        returnX += (myPower(x, j) / myFactorial(j));
    }
	return returnX;
}

// Draws a regular polygon with equal length sides and the same angle at each 
// vertex.  Uses our own math functions rather than math.h.
void drawShape(int x, int y, int sides, int height, int width, unsigned int speed) {
	moveTo(0, 0, 4000);
	int centerX = (x + (x + width)) / 2; // The center of the shape
	int centerY = (y + (y + width)) / 2;
	double lastX = 0; // Keeps track of the last (x,y) we drew so that we can draw a line
	double lastY = 0;
	int initialX = 0; // Keeps track of the first (x,y) to avoid rounding errors
	int initialY = 0;

	for (int i = 0; i <= sides; i++) {
		double vertexX = (width / 2) * myCos(2 * MYPICONST * i / sides) + centerX;
		double vertexY = (width / 2) * mySin(2 * MYPICONST * i / sides) + centerY;
		if (i != 0 && i != sides) { // Draw a line between verticies
			drawLine((int)lastX, (int)lastY, (int)vertexX, (int)vertexY, speed); 
		} else if (i == 0){ // Don't draw the first point calculated, just saves it
			initialX = (int)vertexX;
			initialY = (int)vertexY;
		} else { // When i == sides, draw the last line back to the very first point
			drawLine((int)lastX, (int)lastY, (int)initialX, (int)initialY, speed);
		}
		lastX = vertexX;
		lastY = vertexY;
	}
	penUp();
	moveTo(0, 0, 4000); // Reset
}



