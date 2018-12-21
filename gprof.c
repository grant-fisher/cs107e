#include "armtimer.h"
#include "malloc.h"
#include "console.h"
#include "gprof.h"
#include "interrupts.h"
#include "assert.h"

/* Interval for profiler timer interrupts */
#define GPROF_TIMER_INTERVAL 0x10
#define ON 1
#define OFF 0

extern int text_end;
extern int __bss_start__;
unsigned int __text_end__;
unsigned int __text_start__;


//int pcArr[(__text_end__ - __start_end__)/4];
//int countArr[(__text_end__ - __start_end__)/4];
int textSize; 
int currIndex;
int onOrOff;
int * pcArr;
int * countArr;
 int realPC;

void gprof_handler(unsigned int pc) 
{
  /* FIXME: Increment the count for the given address */
    realPC = pc;
    //count[pc - textstart]++
    if(armtimer_check_and_clear_interrupt()){
        //printf("pc: %x \n", pc);
        countArr[pc - 0x8000]++;
    }

    currIndex++;
}

void gprof_init(void) 
{ 
    armtimer_init(GPROF_TIMER_INTERVAL);
    armtimer_enable();
    armtimer_enable_interrupts();
    //gpio_enable_event_detection(CLK, GPIO_DETECT_FALLING_EDGE);
    interrupts_enable_basic(INTERRUPTS_BASIC_ARM_TIMER_IRQ);
    bool ok = interrupts_attach_handler(gprof_handler); //when interrupt occurs, calls handler
    assert(ok); //check that handler was attacjed, that it worked
    //interrupts_enable_source(INTERRUPTS_GPIO3);
    __text_end__ = (unsigned int)&text_end;
    __text_start__ = 0x8000;
    textSize = __text_end__ - __text_start__;
    onOrOff = OFF;
    currIndex = 0;

}

void gprof_on(void) 
{
    
    
    //armtimer_enable();
    onOrOff = ON;

    pcArr = malloc(textSize/4);
    countArr = malloc(textSize/4);
    currIndex = 0;

    for(int i = 0; i < textSize; i++){
        pcArr[i] = (i*4) + __text_start__;
    }

    for(int i = 0; i < textSize; i++){
         countArr[i] = 0;
    }
}

void gprof_off(void) 
{
    //armtimer_disable();
    interrupts_disable_basic(INTERRUPTS_BASIC_ARM_TIMER_IRQ);
    onOrOff = OFF;

}

bool gprof_is_active(void) 
{
    if(onOrOff){
        return true;
    } else {
        return false;
    }
}

void sort(void){

    printf("currIndex: %d \n", currIndex);
    for (int i = 0; i < 1000; i++) {
        int max = i;
        for (int j = i+1; j < 1000; j++) {
            if (countArr[j] < countArr[max]) max = j;
        }
        int temp = countArr[i];
        //int temp2 = pcArr[i];
        countArr[i] = countArr[max];
        //pcArr[i] = pcArr[max];
        countArr[max] = temp;
        //pcArr[max] = temp2;
        printf("!");
    }
}

// void selectionSort(int[] ar){
//    for (int i = 0; i ‹ ar.length-1; i++)
//    {
//       int min = i;
//       for (int j = i+1; j ‹ ar.length; j++)
//             if (ar[j] ‹ ar[min]) min = j;
//       int temp = ar[i];
//       ar[i] = ar[min];
//       ar[min] = temp;
// } }


void gprof_dump(void) 
{
    // console_printf("     PC     |  COUNT    \n");
    // console_printf("************************\n");
    printf("     PC     |  COUNT    \n");
    printf("************************\n");
    /* FIXME: print out the instructions with 10 highest counts using:
       console_printf("%p %s+%d: %d\n", address, fn_name, offset, count); 
    */

    printf("end: %x \n", __text_end__);
    printf("textSize: %x \n", textSize);
    printf("currIndex: %d \n", currIndex);
    printf("text_end: %d \n", text_end);
    printf("__text_end__: %d \n", __text_end__);
    //printf("bssStart: %d \n", &__bss_start__);

    sort();
    for(int i = 0; i < 10; i++){
        printf("pc: %x ", *(pcArr + i));
        printf("count: %x \n", *(countArr + i));
    }
}
//int * pcArr;
//int * countArr;


// void selectionSort(int[] ar){
//    for (int i = 0; i ‹ ar.length-1; i++)
//    {
//       int min = i;
//       for (int j = i+1; j ‹ ar.length; j++)
//             if (ar[j] ‹ ar[min]) min = j;
//       int temp = ar[i];
//       ar[i] = ar[min];
//       ar[min] = temp;
// } }


