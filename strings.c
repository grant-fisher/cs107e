#include "strings.h"

void *memset(void *s, int c, size_t n) {
    char *current; 
    current = (char *)s; 
    for (int lengthN = 0; lengthN < n; lengthN++) { 
        unsigned char newC = (unsigned char)c; //This chunk of code writes 'c' to s, which increments.
        *current = newC;
        current++;
    }
    return s;
}

void *memcpy(void *dst, const void *src, size_t n) {
    char *currentSRC;
    char *currentDST;
    currentSRC = (char *) src; //Because src and dst are void pointers, have to cast to char pointers.
    currentDST = (char *) dst;
    for(int sizeN = 0; sizeN < n; sizeN++) {
        *currentDST = *currentSRC;
        currentSRC++;
        currentDST++;
    }
    return dst;
}

int strlen(const char *s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) ;
        return i;
}

int strcmp(const char *s1, const char *s2) {
    const char *currentS1 = s1;
    const char *currentS2 = s2;
    int maxLen;

    if(strlen(s1) < strlen(s2)){ //This chunk checks to see what the maximum length that way the function checks all of the possible characters.
        maxLen = strlen(s2);
    } else {
        maxLen = strlen(s1);
    }

    for(int stringLen = 0; stringLen < maxLen; stringLen++) {

        if(*currentS1 == '\0') { //If string 1 is shorter, return -1, ele keep incrementing;
            return -1;
        } else if (*currentS1 > *currentS2) {
            return 1;
        } else if (*currentS1 < *currentS2) {
            return -1;
        }
        currentS1++; //Increment until one is shorter or it finds a different character.
        currentS2++;
    } 
    return 0;
}

int strlcat(char *dst, const char *src, int maxsize){
    const char *currentSRC = src;
    char *currentDST = dst;
    int currentLen = strlen(dst); //Finds starting point to write to dst.
    currentDST += currentLen;

        for (int i = 0; src[i] != '\0'; i++) { 
            if (currentLen + i + 1 <= maxsize - 1) { //Checks to see if there is space left (including one extra space for the null terminating charater).
                *currentDST = *currentSRC;
                currentDST++;
                currentSRC++;
            }
        }
        *currentDST = '\0'; 
        return strlen(dst);
    }

    
unsigned int strtou(const char *str, char **endptr, int base) {
    unsigned int result = 0;
    int checkValid = 1; //When checkValid is turned off (0), strtou stops translating.
    int num;

    while(checkValid) { //This chunk of code reads the digits left to right, incrementing result while checkValid is true;
        num = *str;  
        if((base == 10) && (num >= 48 && num <= 57)) { 
            result *= base;
            result += (num - 48);
        } else if ((base == 16) && (num >= 48 && num <= 57)){
            result *= base;
            result += (num - '0');
        } else if ((base == 16) && (num >= 65 && num <= 70)) {
            result *= base;
            result += (num - 55);
        } else if ((base == 16) && (num >= 97 && num <= 102)){
            result *= base;
            result += (num - 87);
        } else {
            checkValid = 0;
        }
        str++;
    }

    if(endptr != NULL) { 
        str--;
        *endptr = (char *)str; //If the endptr is not NULL, then endptr points to the first invalid character.
    }
    return result;
}
