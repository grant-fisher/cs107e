// This file controls and initializes the servo motor that is attached
// to our pen.  

#include "printf.h"
#include "gpio.h"
#include "pwm.h"
#include "timer.h"
#include <stdbool.h>

#define CLOCK 1000000
#define WIDTH 1500
#define RANGE 20000
#define UPPER 1500 // Constant value to make pen go up
#define LOWER 1000 // Constant value to make pen go down

static bool isUp; // Keeps track of current pen position

// Moves the pen up by sending pwm signals to the servo. If the pen is already
// up, it does nothing.
void penUp(void) {
    if(!isUp){
        pwm_set_width(0, UPPER); 
        timer_delay_ms(50);
        isUp = true; // Note new pen position
    } 
}

// Moves the pen down by sending pwm signals to the servo. If the pen is 
// already down, it does nothing.
void penDown(void){
    if(isUp){
        pwm_set_width(0, LOWER); 
        timer_delay_ms(50);
        isUp = false; // Note new pen position
    }
}

// Initializes the pen. Sets up GPIO pins, range, width, and the clock.  Also 
// initializes PWM.
void pen_init(void) {
    gpio_init();
    gpio_set_function( 18, GPIO_FUNC_ALT5 ); 
    pwm_init();
    pwm_set_clock(CLOCK); //1 hZ, each tick is a micro second
    pwm_set_mode(0, PWM_MARKSPACE);
    pwm_set_fifo(0, 0);
    pwm_enable(0);
    pwm_set_range(0, RANGE);  //pulse every 20 milliseconds (50hz)
    pwm_set_width(0, WIDTH); 
    isUp = false;
    penUp(); // Begin with the pen up
}