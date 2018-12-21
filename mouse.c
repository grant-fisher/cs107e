#include "gpio.h"
#include "gpioextra.h"
#include "interrupts.h"
#include "ringbuffer.h"
#include "mouse.h"
#include "timer.h"
#include "assert.h"
#include "printf.h"

#define CLK GPIO_PIN25
#define DATA GPIO_PIN26

#define CMD_RESET 0xFF
#define CMD_ENABLE_DATA_REPORTING 0xF4

static rb_t *rb;
unsigned char seq;
int count;
int sum;
int first;

static void mouse_write(unsigned char data);
static void mouse_handler(unsigned int pc);
static int mouse_read_polling_scancode(void);

/*Initilizes the mouse by sending it a reset code and waiting for the appropriate responses from mouse.*/
void mouse_init(void) 
{
  first = 1;
  gpio_set_function(CLK, GPIO_FUNC_INPUT);
  gpio_set_pullup(CLK);
  gpio_set_function(DATA, GPIO_FUNC_INPUT);
  gpio_set_pullup(DATA);

  mouse_write(0xFF); // Put mouse in reset mode

  assert( mouse_read_polling_scancode() == 0xFA ); // Read ACK 
  assert( mouse_read_polling_scancode() == 0xAA ); // Read BAT 
  assert( mouse_read_polling_scancode() == 0x00 ); // Read Device ID
  mouse_write(0xF4); // Enable data reporting
  assert( mouse_read_polling_scancode() == 0xFA ); // Read ACK from mouse

  gpio_enable_event_detection(CLK, GPIO_DETECT_FALLING_EDGE);
  bool ok = interrupts_attach_handler(mouse_handler);
  assert(ok); 
  interrupts_enable_source(INTERRUPTS_GPIO3);
  rb = rb_new();
}

/* Uses binary arithmetic to extract information about button presses, x and y offset, and and overflow. */
mouse_event_t mouse_read_event(void) 
{
  mouse_event_t evt;
  int packet;

  if(first){
    packet = mouse_read_scancode(); //BYTE 1
  } else {
    packet = mouse_read_polling_scancode();
  }

  evt.left = (packet) & 1; //Look for button bits
  evt.right = (packet >> 1) & 1;
  evt.middle = (packet >> 2) & 1;

  int xSign = (packet >> 4) & 1;
  int ySign = (packet >> 5) & 1;

  int xOver = (packet >> 6) & 1;
  int yOver = (packet >> 7) & 1;  

  int yMove;
  int xMove;

  //Seems strange at first, but polling seems to be the most reliable way of receiving the scancode 
  //after exiting paint and then reentering it if someone calls "draw" mulitple times in the shell. Most likely some obsure mouse thing.
  if(first){
    xMove = mouse_read_scancode();
  } else {
    xMove = mouse_read_polling_scancode(); 
  }

  if(first){
    yMove = mouse_read_scancode();
  } else {
    yMove = mouse_read_polling_scancode(); 
  }

  if(xSign == 1) {
    xMove = xMove | 0x100; //Add the negative bit to end of x movement int
  } 
  evt.dx = xMove;

  if(ySign == 1) {
    yMove = yMove | 0x100; //Add the negative bit to end of y movement int
  }

  evt.dy = yMove;
  evt.x_overflow = xOver;
  evt.y_overflow = yOver;

  return evt;
}

void wait_for_falling_clock_edge() {
    while (gpio_read(CLK) == 0) {}
    while (gpio_read(CLK) == 1) {}
}

void resetMouse(void){
    seq = 0;
    count = 0;
    sum = 0;
}

static void mouse_handler(unsigned int pc) 
{
  if (gpio_check_and_clear_event(CLK)) {
    unsigned int bit = gpio_read(DATA);
    count++;
   
    if(count == 1){ //gets and checks each bit until full scancode is received
        if(bit == 1){
            resetMouse();
            return; 
        }
    } else if(count >= 2 && count <= 9) { 
        sum += bit;
        seq |= bit << (count - 2);
    } else if (count == 10) {
        if(((sum + bit) % 2) == 0){  
            resetMouse();
            return;
        } 
    } else if(count == 11){

        if(bit == 0) {
            resetMouse();
            return;
        }
        rb_enqueue(rb, seq);
        resetMouse();
    }
  }
}

static int mouse_read_polling_scancode(void)
{
  unsigned char seq = 0;
  while(1) {
    unsigned int bit;
    int numOdd = 0;
    seq = 0;
    wait_for_falling_clock_edge();
    bit = gpio_read(DATA);
    if(bit == 1) { //Check if startbit is correct.
        continue;
    } 
    
    for (int i = 0; i < 8; i++) { 
        wait_for_falling_clock_edge();
        bit = gpio_read(DATA);
        if(bit == 0) {
            numOdd++;
        }
        seq |= bit << i;
    }

    wait_for_falling_clock_edge();
    bit = gpio_read(DATA);

    if(((numOdd + bit) % 2) == 0){  //Add the parity bit and then check if it is still odd.
        continue;
    } 

    wait_for_falling_clock_edge();
    bit = gpio_read(DATA);

    if(bit == 0) {
        continue;
    }
    break;
    }
  return seq; ;  
}

unsigned char mouse_read_scancode(void) 
{
  int sequence = 0;

    while(!rb_dequeue(rb, &sequence)){
        timer_delay_ms(50);
    } 
    return (unsigned char)sequence;
}

//Writes to mouse data specified by the user.
static void mouse_write(unsigned char data) 
{
    int sum = 0;
    gpio_set_output(CLK); //Pull Low
    gpio_set_output(DATA); 

    gpio_write(CLK, 0); //Bring the Clock line low for at least 100 microseconds. 
    timer_delay_us(100);
    gpio_write(DATA, 0); //Bring the Data line low. Startbit
  
    gpio_write(CLK, 1);
    gpio_set_input(CLK); //‘Release’ (set as input again) the clock line 

    for (int i = 0; i < 8; i++) {
      wait_for_falling_clock_edge();
      int bit = (data >> i) & 1;
      sum += bit;
      gpio_write(DATA, bit);
    }
    wait_for_falling_clock_edge();

    if((sum % 2) == 0){  //Add the parity bit.
      gpio_write(DATA, 1);
    } else {
      gpio_write(DATA, 0);
    }

    wait_for_falling_clock_edge();
    gpio_write(DATA, 1); //stop bit
    gpio_set_input(DATA);

    while(gpio_read(DATA) == 1) {}
    while(gpio_read(CLK) == 1) {}
    
}