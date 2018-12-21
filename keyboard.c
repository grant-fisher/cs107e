#include "gpio.h"
#include "gpioextra.h"
#include "keyboard.h"
#include "ps2.h"
#include "interrupts.h"
#include "assert.h"
#include "ringbuffer.h"
#include "timer.h"

const unsigned int CLK  = GPIO_PIN23;
const unsigned int DATA = GPIO_PIN24; 

unsigned int modifierBits = 0;
unsigned int numScrollLock = 0;
unsigned int numNumsLock = 0;
unsigned int numCapsLock = 0;

rb_t* rb;
unsigned char seq; 
int count;
int sum;


void reset(void){
    seq = 0;
    count = 0;
    sum = 0;
}

void keyboard_handler(unsigned int pc){ //called 33 times a keyboard press
    
    if (gpio_check_and_clear_event(CLK)) {
        unsigned int bit = gpio_read(DATA);
        count++;
       
        if(count == 1){ //Check if startbit is correct.
            if(bit == 1){
                reset();
                return; 
            }
        } else if(count >= 2 && count <= 9) {
            sum += bit;
            seq |= bit << (count - 2);
        } else if (count == 10) {
            if(((sum + bit) % 2) == 0){  //Add the parity bit and then check if it is still odd.
                reset();
                return;
            } 
        } else if(count == 11){ 
            if(bit == 0) {
                reset();
                return;
            }
            bool ok = rb_enqueue(rb, seq); 
            assert(ok);
            reset();
        }
    }
}

void keyboard_init(void) 
{
    gpio_set_input(CLK); 
    gpio_set_pullup(CLK); 
 
    gpio_set_input(DATA); 
    gpio_set_pullup(DATA); 

    gpio_enable_event_detection(CLK, GPIO_DETECT_FALLING_EDGE);

    bool ok = interrupts_attach_handler(keyboard_handler); //when interrupt occurs, calls handler
    assert(ok); //check that handler was attacjed, that it worked
    interrupts_enable_source(INTERRUPTS_GPIO3);

    rb = rb_new();

}

//This function goes through the binary and checks to see if it is an acceptable scan code.
unsigned char keyboard_read_scancode(void) 
{
    int sequence = 0;

    while(!rb_dequeue(rb, &sequence)){
        timer_delay_ms(50);
    } 
    
    return (unsigned char)sequence;
}

//This function reads through the scan codes and returns how many codes there are, which sayds if it is extended, up or down.
int keyboard_read_sequence(unsigned char seq[])
{
    seq[0] = keyboard_read_scancode();
        if(seq[0] == 0xE0) {
            seq[1] = keyboard_read_scancode();
            if(seq[1] == 0xF0) {
                seq[2] = keyboard_read_scancode();
                return 3;
            }
            return 2;
        } else if(seq[0] == 0xF0) {
            seq[1] = keyboard_read_scancode();
            return 2;
        } 
        return 1;
}

//This helper function keeps track of number of hits on lock keys, that way they can be set on and off correctly. 
void lock(key_event_t event, int modifierKey, unsigned int numLocks) {
    if(event.action == KEYBOARD_ACTION_DOWN) {
        if(modifierKey == KEYBOARD_MOD_SCROLL_LOCK){
            numScrollLock++;
            numLocks = numScrollLock;
        } else if (modifierKey == KEYBOARD_MOD_NUM_LOCK) {
            numNumsLock++;
            numLocks = numNumsLock;
        } else {
            numCapsLock++;
            numLocks = numCapsLock;
        }
    }
    if ((numLocks % 2) == 0) { //Turn OFF the key.
        modifierBits &= ~modifierKey;
    } else {        
        modifierBits |= modifierKey;  
    }
}

//This helper function modifies the modifer depending on if a key is being held down.
void hold(key_event_t event, int modifierKey) {
    if(event.action == KEYBOARD_ACTION_DOWN){
        modifierBits |= modifierKey; //Held Down
    } else {
        modifierBits &= ~modifierKey; //Not Held
    }
}

key_event_t keyboard_read_event(void) 
{
    key_event_t event;
    event.seq_len = keyboard_read_sequence(event.seq);

    if(event.seq_len >= 2 && (event.seq[0] == 0xF0 || event.seq[1] == 0xF0)) { 
        event.action = KEYBOARD_ACTION_UP;
    } else {
        event.action = KEYBOARD_ACTION_DOWN;
    }
   
    event.key = ps2_keys[event.seq[event.seq_len -1]];
    int lastBits = event.seq[event.seq_len -1];

    if(lastBits == 0x7E) { //This set of if statements checks to see if any of the special mofider keys are being held down.
        lock(event, KEYBOARD_MOD_SCROLL_LOCK, numScrollLock);
    }

    if(lastBits == 0x77) {
        lock(event, KEYBOARD_MOD_NUM_LOCK, numNumsLock); 
    }

    if(lastBits == 0x58){
        lock(event, KEYBOARD_MOD_CAPS_LOCK, numCapsLock);
    }

    if(lastBits == 0x12 || lastBits == 0x59){
        hold(event, KEYBOARD_MOD_SHIFT);
    }

    if(lastBits == 0x11) {
        hold(event, KEYBOARD_MOD_ALT);
    }
    
    if(lastBits ==  0x14) {
        hold(event, KEYBOARD_MOD_CTRL);
    }
    event.modifiers = modifierBits;
    return event;
}

unsigned char keyboard_read_next(void) 
{
     while (1) { 
        key_event_t event = keyboard_read_event();
        if(event.action == KEYBOARD_ACTION_DOWN) {
        int modifier = event.modifiers;
            if(((modifier >> 2 == 1) && (event.key.other_ch >= 0x41 && event.key.other_ch <= 0x5A)) || ((modifier >> 3) == 1)){ //CAPS LOCK
                return event.key.other_ch;
            } else { 
                return event.key.ch;
            }
        }
    }
}
