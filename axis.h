#ifndef AXIS_H
#define AXIS_H

/*
 * initializes theh axis by passing in the gpio pin numbers for the 
 * four x and y stepper directions.
 * param x1: x axis coil 1 direction 1
 * param x2: x axis coil 1 direction 2
 * param x3: x axis coil 2 direction 1
 * param x4: x axis coil 2 direction 2
 * param y1: y axis coil 1 direction 1
 * param y2: y axis coil 1 direction 2
 * param y3: y axis coil 2 direction 1
 * param y4: y axis coil 2 direction 2
*/
void axis_init(int x1, int x2, int x3, int x4, int y1, int y2, int y3, int y4);

/*
 * turns off x coils to stop drawing power
 */
static void x_off(void);

/*
 * turns off y coiils to stop drawing power
 */
static void y_off(void);

/*
 * step x axis stepper motor 1 step in the specified dir
 * param: dir if 1 move forawrd, if 0 backwards
 */
static void x_step(int dir);

/*
 * step y axis stepper motor 1 step in the specified dir
 * param: dir if 1 move forawrd, if 0 backwards
 */
static void y_step(int dir);

/*
 * implements a guessing funciton to find the square root of a number
 * returns the foor of a decimal value
 * param: a - number to be sqrt'd
 * return: floored int sqare root of a
 */
unsigned int int_sqrt(unsigned int a);

/*
 * move along the x axis a number of steps in a passed dir
 * with spd microsecond delay inbetween steps
 * param: step - step number
 * param: dir - direction (1 forward, 0 backward (+- axis))
 */
void move_x(int step, unsigned int dir, unsigned int spd);

/*
 * move along the y axis a number of steps in a passed dir
 * with spd microsecond delay inbetween steps
 * param: step - step number
 * param: dir - direction (1 forward, 0 backward (+- axis))
 */
void move_y(int step, unsigned int dir, unsigned int spd);

/*
 * move in a straight line dx, dy step amounts at a constant speed spd
 * param: dx - number of steps in the x direction (can be negative)
 * param: dy - number of steps in the y direction (can be negative)
 * param: spd - number of micro seconds delay between one step distance
 *      in the direction vector of (dx, dy)
 */
void move(int dx, int dy, unsigned int spd);

#endif