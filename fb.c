#include "mailbox.h"
#include "fb.h"

// This prevents the GPU and CPU from caching mailbox messages
#define GPU_NOCACHE 0x40000000

typedef struct {
  unsigned int width;       // width of the display
  unsigned int height;      // height of the display
  unsigned int virtual_width;  // width of the virtual framebuffer
  unsigned int virtual_height; // height of the virtual framebuffer
  unsigned int pitch;       // number of bytes per row
  unsigned int depth;       // number of bits per pixel
  unsigned int x_offset;    // x of the upper left corner of the virtual fb
  unsigned int y_offset;    // y of the upper left corner of the virtual fb
  unsigned int framebuffer; // pointer to the start of the framebuffer
  unsigned int size;        // number of bytes in the framebuffer
} fb_config_t;

// Fb is volatile because the GPU will write to it
static volatile fb_config_t fb __attribute__ ((aligned(16))); 

void fb_init(unsigned int width, unsigned int height, unsigned int depth, unsigned int mode)
{
  fb.width = width;
  fb.virtual_width = width;
  fb.height = height;

  if(mode == FB_DOUBLEBUFFER){
    fb.virtual_height = height*2; //For double-buffering, program switches this to x2 as large to get more space.
  } else if (mode == FB_SINGLEBUFFER){
    fb.virtual_height = height;
  }
  
  fb.depth = depth * 8; // Convert number of bytes to number of bits
  fb.x_offset = 0;
  fb.y_offset = 0;

  fb.pitch = 0;
  fb.framebuffer = 0;
  fb.size = 0;

  mailbox_write(MAILBOX_FRAMEBUFFER, (unsigned)&fb + GPU_NOCACHE);
  (void) mailbox_read(MAILBOX_FRAMEBUFFER);
}

//This function swaps the y-offset from top to bottom or vise versa.
void fb_swap_buffer(void)
{
  if(fb.virtual_height == fb.height) return; //If it is single buffer, do not swap buffer.

  if(fb.y_offset == 0){ 
    fb.y_offset = fb.height;
  } else {
    fb.y_offset = 0;
  }
  mailbox_write(MAILBOX_FRAMEBUFFER, (unsigned)&fb + GPU_NOCACHE);
  (void) mailbox_read(MAILBOX_FRAMEBUFFER);
}

//This function returns the buffer which is not being shown so that it can update the drawing.
unsigned char* fb_get_draw_buffer(void) 
{
  if(fb.virtual_height == fb.height) return (unsigned char*)(fb.framebuffer); //Acounts for Single buffering mode

  if(fb.y_offset == 0){ //This if/else segment switches which buffer is being draw to.
    return (unsigned char*)(fb.framebuffer + (fb.pitch * (fb.height))); // Returns frame not being shown.
  } else {
    return (unsigned char*)(fb.framebuffer);
  }
}

unsigned int fb_get_width(void)
{
  return fb.width;
}

unsigned int fb_get_height(void)
{
  return fb.height;
}

unsigned int fb_get_depth(void)
{
  return fb.depth;
}

unsigned int fb_get_pitch(void)
{
  return fb.pitch;
}

