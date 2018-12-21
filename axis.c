
#include "axis.h"
#include "gpio.h"
#include "timer.h"
#include "printf.h"
#include "pwm.h"

#include <math.h>

#define FORWARD 1;
#define BACKWARD 0;

static int XCOILPIN1;
static int XCOILPIN2;
static int XCOILPIN3;
static int XCOILPIN4;

static int YCOILPIN1;
static int YCOILPIN2;
static int YCOILPIN3;
static int YCOILPIN4;

static unsigned int x_step_num;	//counters to keep track of cycle number while stepping
static unsigned int y_step_num;

/*
 * turns off x coils to stop drawing power
 */
static void x_off(void) {
	gpio_write(XCOILPIN1, 0);
	gpio_write(XCOILPIN2, 0);
	gpio_write(XCOILPIN3, 0);
	gpio_write(XCOILPIN4, 0);
}

/*
 * turns off y coiils to stop drawing power
 */
static void y_off(void) {
	gpio_write(YCOILPIN1, 0);
	gpio_write(YCOILPIN2, 0);
	gpio_write(YCOILPIN3, 0);
	gpio_write(YCOILPIN4, 0);
}

/*
 * step x axis stepper motor 1 step in the specified dir
 * param: dir if 1 move forawrd, if 0 backwards
 */
static void x_step(int dir) 
{
	//half steps - cycles defined below based off of step number counter
	gpio_write(XCOILPIN1, x_step_num == 0 || x_step_num == 1 || x_step_num == 7);
	gpio_write(XCOILPIN2, x_step_num >= 3 && x_step_num <= 5);
	gpio_write(XCOILPIN3, x_step_num >= 1 && x_step_num <= 3);
	gpio_write(XCOILPIN4, x_step_num >= 5 && x_step_num <= 7);

	//either adds or subtracts one from step number, modulos 8 to wrap around
	x_step_num = (x_step_num + (dir ? 1 : -1)) % 8;

}

/*
 * step y axis stepper motor 1 step in the specified dir
 * param: dir if 1 move forawrd, if 0 backwards
 */
static void y_step(int dir)
{
	//half steps - cycles defined below based off of step number counter
	gpio_write(YCOILPIN1, y_step_num == 0 || y_step_num == 1 || y_step_num == 7);
	gpio_write(YCOILPIN2, y_step_num >= 3 && y_step_num <= 5);
	gpio_write(YCOILPIN3, y_step_num >= 1 && y_step_num <= 3);
	gpio_write(YCOILPIN4, y_step_num >= 5 && y_step_num <= 7);

	//either adds or subtracts one from step number, modulos 8 to wrap around
	y_step_num = (y_step_num + (dir ? 1 : -1)) % 8;
}

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
void axis_init(int x1, int x2, int x3, int x4, int y1, int y2, int y3, int y4) 
{
	x_step_num = 0;
	y_step_num = 0;

	XCOILPIN1 = x1;
	XCOILPIN2 = x2;
	XCOILPIN3 = x3;
	XCOILPIN4 = x4;
	
	YCOILPIN1 = y1;
	YCOILPIN2 = y2;
	YCOILPIN3 = y3;
	YCOILPIN4 = y4;

	gpio_set_output(x1);
	gpio_set_output(x2);
	gpio_set_output(x3);
	gpio_set_output(x4);

	gpio_set_output(y1);
	gpio_set_output(y2);
	gpio_set_output(y3);
	gpio_set_output(y4);
}

/*
 * move along the x axis a number of steps in a passed dir
 * with spd microsecond delay inbetween steps
 * param: step - step number
 * param: dir - direction (1 forward, 0 backward (+- axis))
 */
void move_x(int step, unsigned int dir, unsigned int spd)
{

	for (int i = 0; i < step; i++) {
		x_step(dir);
		timer_delay_us(spd);
	}
	x_off();
}

/*
 * move along the y axis a number of steps in a passed dir
 * with spd microsecond delay inbetween steps
 * param: step - step number
 * param: dir - direction (1 forward, 0 backward (+- axis))
 */
void move_y(int step, unsigned int dir, unsigned int spd)
{

	for (int i = 0; i < step; i++) {
		y_step(dir);
		timer_delay_us(spd);
	}
	y_off();
}

/*
 * implements a guessing funciton to find the square root of a number
 * returns the foor of a decimal value
 * param: a - number to be sqrt'd
 * return: floored int sqare root of a
 */
unsigned int int_sqrt(unsigned int a) {
	//base cases
	if (a == 0 || a == 1)
		return a;

	int start = 1;
	int end = a;
	int ans;
	while (start <= end) {
		int mid = (start + end) / 2;

		if (mid * mid == a)
			return mid;

		if (mid * mid < a) {
			start = mid + 1;
			ans = mid;
		} else {
			end = mid - 1;
		}
	}
	return ans;
}

/*
 * move in a straight line dx, dy step amounts at a constant speed spd
 * param: dx - number of steps in the x direction (can be negative)
 * param: dy - number of steps in the y direction (can be negative)
 * param: spd - number of micro seconds delay between one step distance
 *      in the direction vector of (dx, dy)
 */
void move(int dx, int dy, unsigned int spd) {
	//gets direction of x and y motion
	unsigned int xdir = dx > 0;
	unsigned int ydir = dy > 0;
	//if negative flip dx and dy
	if (!xdir)
		dx = -dx;

	if (!ydir)
		dy = -dy;

	//total delay between point a and b 
	unsigned int total_delay = spd * int_sqrt(dx*dx+dy*dy);
	//calculate the delay between each step in the x and y direction
	unsigned int xspd = total_delay / dx;
	unsigned int yspd = total_delay / dy;
	//delay is used to calculate the delay remaining between each step in the x and y direciton
	unsigned int xdelay = xspd;
	unsigned int ydelay = yspd;
	// while there are steps to take in both x and y direction
	while (dx > 0 && dy > 0) {
		//if there is less to delay between x steps than y steps
		if (xdelay < ydelay) {
			//step x and decrease y's delay accordingly
			x_step(xdir);
			dx--;
			timer_delay_us(xdelay);
			ydelay -= xdelay;
			xdelay = xspd;
		//if the next step is a y step
		} else if (xdelay > ydelay) {
			//step y and decrease x's delay accoridngly
			y_step(ydir);
			dy--;
			timer_delay_us(ydelay);
			xdelay -= ydelay;
			ydelay = yspd;
		//if both steps happen at the same time
		} else if (xdelay == ydelay) {
			//step both x and y and reset delays
			x_step(xdir);
			dx--;
			y_step(ydir);
			dy--;
			timer_delay_us(xdelay);
			xdelay = xspd;
			ydelay = yspd;
		}
	}
	//finish x steps
	while (dx > 0) {
		x_step(xdir);
		dx--;
		timer_delay_us(xspd);
	}
	x_off();
	//finish y steps
	while (dy > 0) {
		y_step(ydir);
		dy--;
		timer_delay_us(yspd);
	}
	y_off();
}