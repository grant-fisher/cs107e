#include "gpio.h"

volatile unsigned int * const FSEL0 =  (volatile unsigned int *)0x20200000; //Pins 0-9
volatile unsigned int * const FSEL1 =  (volatile unsigned int *)0x20200004; //10-19
volatile unsigned int * const FSEL2 =  (volatile unsigned int *)0x20200008; //20-29
volatile unsigned int * const FSEL3 =  (volatile unsigned int *)0x2020000C; //30-39
volatile unsigned int * const FSEL4 =  (volatile unsigned int *)0x20200010; //40-49
volatile unsigned int * const FSEL5 =  (volatile unsigned int *)0x20200014; //50-59

volatile unsigned int * const SET0  =  (volatile unsigned int *)0x2020001c;
volatile unsigned int * const CLR0  =  (volatile unsigned int *)0x20200028;

volatile unsigned int * const SET1  =  (volatile unsigned int *)0x20200020;
volatile unsigned int * const CLR1  =  (volatile unsigned int *)0x2020002C;

volatile unsigned int * const LEV0  =  (volatile unsigned int *)0x20200034;
volatile unsigned int * const LEV1  =  (volatile unsigned int *)0x20200038;


void gpio_init(void) {

}

//Helper function to determine which FSEl register to read from or write to. Returns a pointer to the address. 
volatile unsigned int * chooseFSEL(unsigned int pin) {
	if(pin < 10 && 0 <= pin) {
			return FSEL0;
		} else if (pin <= 19) {
			return FSEL1;
		} else if (pin <= 29) {
			return FSEL2;
		} else if (pin <= 39) {
			return FSEL3;
		} else if (pin <= 49) {
			return FSEL4;
		} else if (pin <= 53) {
			return FSEL5;
		} else {
			return 0;
		}
}

void gpio_set_function(unsigned int pin, unsigned int function) {

	volatile unsigned int * currentRegister;

	if((pin >= 0 && pin <= 53) && (function >=0 && function <= 7)){

		currentRegister = chooseFSEL(pin);

		int shift = 3*(pin % 10); 
		*currentRegister = ((function << shift) | ((*currentRegister) & ~(0x7 << shift)));
		//To get the correct register, the program masks the register with the inverse of 7 shifted to the correct location
		// and then writes new function in that location
	}
}

unsigned int gpio_get_function(unsigned int pin) {

	volatile unsigned int * currentRegister = 0; 

	if(pin >= 0 && pin <= 53){
		currentRegister = chooseFSEL(pin);
	}

	int newRegister = *currentRegister >> (3*(pin % 10)); 
	newRegister = newRegister & 0x7;   //This chunk of code shifts the bits over until it gets to the right pin, then masks it with 7 to get the bits at that location. 

    return newRegister;  //Then it returns whichever funciton was written for that pin.
}

void gpio_set_input(unsigned int pin) {
    gpio_set_function(pin, 0);
}

void gpio_set_output(unsigned int pin) {
    gpio_set_function(pin, 1);
}

void gpio_write(unsigned int pin, unsigned int value) {
    int registerNum = pin / 32; 
    int shift;

		if (pin >= 0 && pin <54) { //This chunk of if/else statments finds which SET or CLR register to write and set. 
	    	if(registerNum == 0){
	    		shift = 1 << pin;
	    		if(value == 0) {
	    			*CLR0 = shift;
	    		} else if (value == 1){
	    			*SET0 = shift;
	    		}
	    	} else if (registerNum == 1) {
	    		shift = 1 << (pin % 32);
	    		if(value == 0) {
					*CLR1 = shift;
	    		} else if (value == 1){
	    			*SET1 = shift;
	    		}
	    	}
		}
		
}

unsigned int gpio_read(unsigned int pin) {
	int shiftedLEV = 0;
	if(pin >= 0 && pin < 54){        //This chunk of code shits the LEV register the correct number of bits, isolates the least-signifigant bit, then returns it. 
		if (pin >= 0 && pin < 32) {
			shiftedLEV = (*LEV0 >> pin) & 0x1;
		} else if (pin >= 32 && pin < 54) {
			shiftedLEV = *LEV1 >> (pin % 32); 
			shiftedLEV = shiftedLEV & 0x1;
		}
		return shiftedLEV;
	}
	return GPIO_INVALID_REQUEST;
}
