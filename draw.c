#include "printf.h"
#include "uart.h"
#include "interrupts.h"
#include "gl.h"
#include "mouse.h"
#include "console.h"
#include "gpio.h"
#include "ringbufferProject.h"
#include "assert.h"
#include "graphics.h"
#include "axis.h"
#include "pen.h"

#define _WIDTH 450
#define _HEIGHT 450

int oldX;
int oldY;
rb_Project_t* rbX;
rb_Project_t* rbY;
int first;

#define GL_GRAY  0xFF080808
#define RELEASED -1000
#define SPEED 4000

/*Gets a mouse event and instructs the mouse on what to do based on the info stored in the struct*/
void paint(void){
  first = 0;
  rbX = rb_Project_new();
  rbY = rb_Project_new();
  oldX = 0; //initialize cursor to (0,0)
  oldY = 0;

  int newX = 0;
  int newY = 0;
  int notClick = 0;

  while(1){ 
    mouse_event_t event = mouse_read_event();

    if(notClick) { //Erase cursor
      gl_draw_rect(oldX, oldY, 2, 2, GL_WHITE);
    }
  
    int negativeX = (event.dx >> 8) & 1; //Check if negative bit is set
    if(negativeX) {
      newX = ((~(event.dx)) | 1) & 0x1FF; //Deals with the 2's complement of the x movement
      newX = newX * (-1);
    } else {
      newX = event.dx;
    }

    int negativeY = (event.dy >> 8) & 1; //Again follows the previous process for Y
    if(negativeY) {
      newY = ((~(event.dy)) | 1)& 0x1FF; //Again, works with twos complement
    } else {
      newY = event.dy;
      newY = newY * (-1);
    }

    if(!event.x_overflow && !event.y_overflow) { 
      if(event.right){ //Signals exit from program
        break;
      } 
      if(event.left) { 
        gl_draw_rect(oldX + newX, oldY + newY, 2, 2, GL_BLACK);
        notClick = 0; //is clicking
        bool okX = rb_Project_enqueue(rbX, oldX + newX); //Enqueue the new coordinates into ringbuffers.
        assert(okX);
        bool okY = rb_Project_enqueue(rbY, oldY + newY); 
        assert(okY);
      } else {
        gl_draw_rect(oldX + newX, oldY + newY, 2, 2, GL_GRAY); //Draws cursor.
        if(!notClick) { //is being clicked  
          bool okX1 = rb_Project_enqueue(rbX, RELEASED);  
          assert(okX1);
          bool okY1 = rb_Project_enqueue(rbY, RELEASED);
          assert(okY1);
        }
        notClick = 1;
      }
      oldX += newX;
      oldY += newY;
    }
  }
}

/*After the paint ringbuffer stores all the x and y values, the draw function draws lines from the appropriate coordinations to create the image.*/
void draw(void){
  int prevX = 0;
  int prevY = 0;
  int currX = 0;
  int currY = 0;
  int release = 1;

  bool ok1 = rb_Project_dequeue(rbX, &prevX);
  assert(ok1);

  bool ok2 = rb_Project_dequeue(rbY, &prevY);
  assert(ok2);

  while(rb_Project_dequeue(rbX, &currX) && rb_Project_dequeue(rbY, &currY)){ //Go through until empty
    if(currX != RELEASED && currX != RELEASED){ //If mouse was being dragged.
      if(release) { //If it was previously released, create a new line.
        ok1 = rb_Project_dequeue(rbX, &prevX);
        assert(ok1);
        rb_Project_dequeue(rbY, &prevY);
        assert(ok2);
        if(prevX == RELEASED || prevY == RELEASED){ //Accounts for single click edge case
          prevX = currX;
          prevY = currY;
        }
        drawLine(prevX, prevY, currX, currY, SPEED); //Draws line from new point, if the mouse has just been released
        release = 0;
      } else { 
        drawLine(prevX, prevY, currX, currY, SPEED); 
        prevX = currX;
        prevY = currY;
      }
    } else if(currX == RELEASED && currX == RELEASED && prevX != RELEASED && prevY != RELEASED) { 
      release = 1; //the click was released
      penUp();
    } 
  }
}

//Initializes all compenents needed to draw, then calls draw and paint.
void image(void){
  first = 1;
  moveTo(0, 0, SPEED);
  gpio_init();
  gl_init(_WIDTH, _HEIGHT, GL_SINGLEBUFFER);
  gl_clear(GL_WHITE);
  mouse_init();
  paint();
  draw();
  penUp();
  moveTo(0, 0, SPEED);
}
