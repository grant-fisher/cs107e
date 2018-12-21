#ifndef DRAW_H
#define DRAW_H

/*Gets a mouse event and instructs the mouse on what to do based on the info stored in the struct*/
void paint(void);

/*After the paint ringbuffer stores all the x and y values, the draw function draws lines from the appropriate coordinations to create the image.*/
void draw(void);

//Initializes all compenents needed to draw, then calls draw and paint.
void image(void);

#endif