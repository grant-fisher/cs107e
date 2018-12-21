#include "gpio.h"
#include "gpioextra.h"
#include "keyboard.h"
#include "console.h"
#include "shell.h"
#include "interrupts.h"
#include "printf.h"
#include "ringbuffer.h"

#define NROWS 20
#define NCOLS 40

void main(void) 
{
	keyboard_init();
    console_init(NROWS, NCOLS);
   	shell_init(console_printf, console_putchar);
    interrupts_global_enable();
	shell_run();
}