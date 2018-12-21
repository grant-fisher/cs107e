#include "console.h"
#include "gl.h"
#include "printf.h"
#include "malloc.h"
#include "strings.h"

#define MAX_OUTPUT_LEN 1024

int rows = 0;
int cols = 0;
int xPos = 0;
int rowsPassed = 0;
char * cursor;
char * oldCursor;
int cursorCount = 0;

//Initlizes a framebuffer of specified rows and ncols based on char size and initilzes values for global variables.
void console_init(unsigned int nrows, unsigned int ncols)
{
	gl_init(ncols*gl_get_char_width(), nrows*gl_get_char_height(), GL_DOUBLEBUFFER);
	gl_clear(GL_WHITE);
    cursor = (char *)malloc(nrows*ncols);
    oldCursor = cursor;
    xPos = 0; 
    rows = nrows;
    cols = ncols;
}

//Helper function to print out the Buffer.
void printBuffer(char * pointer){
	for(int i = 0; i < (rows*cols); i++){
		printf("i: %c \n", *(pointer+i));
	}
}

//Refresh clears, draws, and then shows the new buffer.
void refresh(char * oldCursor, int index){
	gl_clear(GL_WHITE);
	for(int i = 0; i < index; i++) {
		gl_draw_char((i%cols)*gl_get_char_width(), (i/cols)*gl_get_char_height(), *(oldCursor+i), GL_BLACK);
	}
	gl_swap_buffer();
}

//Clears the textbuffer, and resets the grpahics and number of rows passed.
void console_clear(void)
{
	for(int i = 0; i < rows; i++){
   		for(int j = 0; j < cols; j++){
    		oldCursor[(i*cols) + j] = ' ';
    	}
    }
    rowsPassed = 0;
    xPos = 0;
    cursor = oldCursor;
    refresh(oldCursor, rows*cols);
}

//This function updates the textbuffer so that it displays the new text.
int console_putchar(int ch)
{	
	if(ch == '\n'){
		for (int i = 0; i < (cols - xPos); i++){
			*cursor = ' ';
			cursor++;
			}
		rowsPassed++;
		xPos = 0;
		return ch;
	} else if (ch == '\b'){
		if(((xPos)%cols)!= 0){
			cursor--; 
			*cursor = ' ';
			xPos--;
			refresh(oldCursor, (rowsPassed * cols) + xPos);
		}
		return ch;
	} else if (ch == '\f') {
		console_clear();
		return ch;
	} else if(ch != 7){
		if(xPos == cols){
			xPos = 0;
			rowsPassed++;
		}
		if(rowsPassed >= rows) { //This chunk of code shifts the previous text up one row, if screen is fileld.
			char* beginBuf = (char *)memcpy(oldCursor, oldCursor+(cols), (rows-1)*cols);
			rowsPassed -= 1;
			cursor -= cols;
			for(int i = 0; i < cols; i++){ //Clears the last line of the text buffer.
				*(beginBuf + (rows - 1)*cols) = ' ';
				beginBuf++;
			}
		} 
		*cursor = ch;
		refresh(oldCursor, ((rowsPassed +1)*cols) + xPos); //fix this
		xPos++;
		cursor++;
	}
	return ch;
}

int console_printf(const char *format, ...)
{
	char buf[MAX_OUTPUT_LEN];
	va_list args;
	va_start(args, format);
	int returnVal = vsnprintf(buf, MAX_OUTPUT_LEN, format, args);
	for(int i = 0; i < returnVal; i++){
		console_putchar(buf[i]);
    }
    va_end(args);
    return returnVal;
}
