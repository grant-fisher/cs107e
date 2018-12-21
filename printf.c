#include "printf.h"
#include "strings.h"
#include "uart.h"
#include <stdarg.h>

#define MAX_OUTPUT_LEN 1024
#define POINTER_SIZE 8

//This helper function digitCounter returns how many digits places are in a number.
int digitCounter(unsigned int number, int base) {
    int digit = 0;
    if(number <= 0) { //If number is negative, increment digit once to account for the negative sign.
        digit++; 
        number *= -1;
    }
    while (number > 0) {
        number /= base;
        digit++;
    }
    return digit;
}

//findDivisor uses a number's base and takes in a power, to find out what to divide the number by to get only the most signifigant digit.
int findDivisor(int base, int power) {
    int newDiv = 1;
    for(int i = 0; i < (power - 1); i++) {
        newDiv *= base;
    }
    return newDiv;
}

int unsigned_to_base(char *buf, int bufsize, unsigned int val, int base, int min_width) {
    int digits = digitCounter(val, base);
    int returnInt = 0; //Return int keeps track of number of characters that would have been written if there were room.
    bufsize--;
    if(digits < min_width) { //If the min_width is larger than the number of digits in Val, add extra zeros infront. 
        for(int i = 0; i < (min_width - digits); i++){
            returnInt++;
            if(bufsize != 0) {
                *buf = '0';
                buf++;
                bufsize--;
            }
        }
    }
    returnInt += digits; 
    while(digits > 0 && bufsize > 0) {
        unsigned int currentDivider = findDivisor(base, digits);        
        unsigned int currDigit = val / currentDivider;
        val = val % currentDivider;
        digits--;
        
        if(currDigit <= 9 && currDigit >= 0){ //This chunk of code determines whether the current digit is hex or dec and writes to buf.
            *buf = currDigit + '0';
        } else if (currDigit <= 15 && currDigit >= 10 ) {
            *buf = currDigit + 87;
        }
        buf++;
        bufsize--;
    }
    *buf = '\0';
    return returnInt;
}

int signed_to_base(char *buf, int bufsize, int val, int base, int min_width) {
    *buf = '-';
    buf++;
    if(val < 0) {
        val *= -1;
    }
    int returnInt = unsigned_to_base(buf, bufsize - 1, val, base, min_width -1);
    return returnInt + 1;
}

int vsnprintf(char *buf, int bufsize, const char *format, va_list args) {
    int full = 0; //The full variable switches vsnprintf from writing to just reading, inorder to find out how many characters would have been written if bufsize did not runout. 
    int returnVal = 0;
   
if(bufsize != 0) {
    bufsize--; //Decrement bufsize once at beginning to account for null terminating character.
    while( *format != '\0') { 
        if (bufsize <= 0 ){
            full = 1;
        }
        if(*format == '%') {
            format++;
            char width[bufsize]; //Width holds the min_width size.
            char * widthPointer = width;
            int minWidth = 0;
            
            //This chunk of code gets the string containing the width, and then translates it to int. 
            while(*format != 'p' && *format != 'd' && *format != 'c' && *format != 'd' && *format != 'x' && *format != 's'){ 
                *widthPointer = *format;
                widthPointer++;
                format++;
                *widthPointer = '\0';
                minWidth = strtou(width, '\0', 10);
            }

            if(*format == 'c'){ 
                int character = va_arg(args, int);
                if(!full){
                    *buf = (char) character;
                    buf++;
                    bufsize--;
                    *buf = '\0'; 
                } 
                returnVal++;
            }    
            if(*format == 's') { 
                char* stringArg = va_arg(args, char*); 
                if(!full){
                    *buf = '\0';
                    strlcat(buf, stringArg, bufsize + 1); 
                    if(strlen(stringArg) <= bufsize){ //Checks if the length of string is less than remaining buff size so buf doesnt point to off-buffer memory
                        buf += strlen(stringArg);
                    }
                }
                returnVal += strlen(stringArg);
            }
            if(*format == 'd' || *format == 'x') { 
                int intArg = va_arg(args, int);
                int base = 0;
                if(*format == 'd') {
                    base = 10;
                } else {
                    base = 16;
                }

                if(intArg < 0 && (*format == 'd')) {
                    signed_to_base(buf, bufsize + 1, intArg, base, minWidth); 
                } else {
                    unsigned_to_base(buf, bufsize + 1, intArg, base, minWidth);  
                }
                if(minWidth > digitCounter(intArg, base)){ 
                    if(!full){
                        buf += minWidth;
                        bufsize -= minWidth; 
                    }
                    returnVal +=minWidth;
                } else {
                    if(!full){
                        if(digitCounter(intArg, base) <= bufsize){  //Checks remaining bufsize so that buf doesn't point to off-buffer memory.
                            buf += digitCounter(intArg, base);
                        } 
                        bufsize -= digitCounter(intArg, base);
                    }
                    returnVal += digitCounter(intArg, base);
                }
            }

            if(*format == 'p'){
                if(!full) {
                    *buf = '0';
                    buf++;
                    bufsize--;
                    if (bufsize > 0) { //Checks to see if 0 was written to last space, and x cannot be written.
                        *buf = 'x';
                        buf++;
                        bufsize--;
                    }
                    unsigned_to_base(buf, bufsize, va_arg(args, int), 16, POINTER_SIZE);
                    buf += POINTER_SIZE;
                    bufsize -= POINTER_SIZE;
                }
                returnVal += 10;
            }
            format++;
        } else if (*format != '\0') { //If it is not a format code, simply write the character.
            if(!full){
                *buf = *format;
                buf++;
                bufsize--; 
            }
            returnVal++;
            format++;
        }
    }
    *buf = '\0';
}
    return returnVal;
}

int snprintf(char *buf, int bufsize, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int returnVal = vsnprintf(buf, bufsize, format, ap); 
    va_end(ap);
    return returnVal;
}

int printf(const char *format, ...) {
    char buf[MAX_OUTPUT_LEN];
    memset(buf, '\0', MAX_OUTPUT_LEN);
    va_list args;
    va_start(args, format);
    int returnVal = vsnprintf(buf, MAX_OUTPUT_LEN, format, args);

    for(int i = 0; i < returnVal; i++){
        uart_putchar(buf[i]);
    }
    va_end(args);
    return returnVal; 
}
