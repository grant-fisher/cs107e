#include "gl.h"
#include "font.h"
#include "fb.h"
#include "strings.h"

#define GL_GRAY   0xFF707070
#define GL_LIGHTGRAY   0xFFA9A9A9
#define GL_CLOUD   0xFFD3D3D3

//This function initializes a framebuffer of the appropriate size and mode.
void gl_init(unsigned int width, unsigned int height, unsigned int mode)
{
    fb_init(width, height, 4, mode); 
}

void gl_swap_buffer(void)
{
    fb_swap_buffer();
}

unsigned int gl_get_width(void) 
{
    return fb_get_width();
}

unsigned int gl_get_height(void)
{
    return fb_get_height();
}

//This function returns the color of the pixel depending on the RGB values.
color gl_color(unsigned char r, unsigned char g, unsigned char b)
{
    int alpha = 0xff;
    color pixColor = 0;
    pixColor = r << 16 | g << 8 | b | alpha << 24;
    return pixColor;
}

//Clears the monitor to one color.
void gl_clear(color c)
{
    gl_draw_rect(0, 0, gl_get_width(), gl_get_height(), c);
}

//Changes a pixel to the color specified.
void gl_draw_pixel(int x, int y, color c)
{
    unsigned(*fbPointer)[fb_get_pitch()/4] = (unsigned(*)[fb_get_pitch()/4])fb_get_draw_buffer();  //make it the pitch 
    if(x < gl_get_width() && y < gl_get_height()){
        fbPointer[y][x] = c;
    }
}

//This funciton checks the color of a pixel.
color gl_read_pixel(int x, int y)
{
    unsigned(*fbPointer)[fb_get_pitch()/4] = (unsigned(*)[fb_get_pitch()/4])fb_get_draw_buffer();
    unsigned int pixColor = 0;
    if(x < gl_get_width() && y < gl_get_height()){
        pixColor = fbPointer[y][x];
    }
    return pixColor;
}

//This funciton draws a rectangle with speficied height and width at x,y location.
void gl_draw_rect(int x, int y, int w, int h, color c)
{
    unsigned(*fbPointer)[fb_get_pitch()/4] = (unsigned(*)[fb_get_pitch()/4])fb_get_draw_buffer();
    for(int row = y; row < y + h; row ++){
        for(int col = x; col < x + w; col ++){
            if(row < gl_get_height() && col < gl_get_width()){
                fbPointer[row][col] = c;
            }
        }
    }
}

//This function draws a character from a bit pattern stored in a buf returned by font_get_char.
void gl_draw_char(int x, int y, int ch, color c)
{
    unsigned char buf[gl_get_char_height()*gl_get_char_width()]; 
    if (font_get_char(ch, buf, sizeof(buf)) == 0){
        for(int height = 0; height < gl_get_char_height(); height++){ 
            for(int width = 0; width < gl_get_char_width(); width++){
                if((x+(width)) <= gl_get_width() && (y+height) <= gl_get_height()){
                    if(buf[((height*gl_get_char_width())+width)] != 0){ //if not a o
                        gl_draw_pixel(x+width, y+height, c);
                    }
                }
            }
        }
    }
}

//This function draws a string of chars using gl_draw_char.
void gl_draw_string(int x, int y, char* str, color c)
{
    for(int i = 0; i < strlen(str); i++){
        if(x+((i+1)*gl_get_char_width()) <= gl_get_width()){
            gl_draw_char(x+(i*gl_get_char_width()), y, *(str + i), c);
        }
    }
}

//Returns height of a character.
unsigned int gl_get_char_height(void)
{
    return font_get_height();  
}

//Returns width of a character.
unsigned int gl_get_char_width(void)
{
    return font_get_width(); 
}

//This funciton finds the absolute value of a double.
double abs (double n)
{
    if(n > 0) return n;
    else return n*(-1);
}

//Floors the double value.
double floor(double x){
    return (int)x;
}

//Rounds the x to closest number.
double round(double x) {
    return floor(x + 0.5);
}

//Finds the fractional part of the number.
double frac(double x){
    return x - (int)x;
}

//Finds how much more a double needs increase to be equal to one.
double rfpart(double x){
    return 1 - frac(x);
}

color grad(double gradVal) {
    if(0 <= gradVal && gradVal <= 0.25) {
        return GL_BLACK;
    } else if (0.25 < gradVal && gradVal <= 0.5){
        return GL_GRAY;
    } else if (0.5 < gradVal && gradVal <= 0.75) {
        return GL_LIGHTGRAY;
    } else {
        return GL_CLOUD;
    }
}

//Draws a line from x to y according to points given.
void gl_draw_line(int x0, int y0, int x1, int y1, color c){
    int steep = 0;

    if(abs(y1 - y0) > abs(x1 - x0)){
        steep = 1;
    }

    if(steep){
        double temp = 0; //SWAP X and Y values
        temp = y0;
        y0 = x0;
        x0 = temp;

        temp = y1;
        y1 = x1;
        x1 = temp;
    }
    if(x0 > x1){
        double temp = x1;
        x1 = x0;
        x0 = temp;

        temp = y1;
        y1 = y0;
        y0 = temp;
    }

    double dx = x1 - x0;
    double dy = y1 - y0;
    double gradient = (double)dy/ (double)dx; //Finds the gradient, degree of brightness
    if(dx == 0){
        gradient = 1;
    }

    double xend = round(x0);
    double yend = y0 + gradient * (xend - x0);
    double xgap = rfpart(x0 + 0.5);
    double xpxl1 = xend;
    double ypxl1 = floor(yend);

    if(steep){
        gl_draw_pixel(ypxl1, xpxl1, grad(rfpart(yend) * xgap)); //Draws first endpoint
        gl_draw_pixel(ypxl1+1, xpxl1, grad(frac(yend) * xgap));
    } else {
        gl_draw_pixel(ypxl1, xpxl1, grad(rfpart(yend) * xgap));
        gl_draw_pixel(ypxl1, xpxl1+1, grad(frac(yend) * xgap));
    }

    double intery = yend + gradient;

    
    xend = round(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = rfpart(x1 + 0.5);
    double xpxl2 = xend;
    double ypxl2 = floor(yend);

    if(steep){
        gl_draw_pixel(ypxl2, xpxl2, grad(rfpart(yend) * xgap)); ////Draws second endpoint
        gl_draw_pixel(ypxl2+1, xpxl2, grad(frac(yend) * xgap));
    } else {
        gl_draw_pixel(ypxl2, xpxl2, grad(rfpart(yend) * xgap));
        gl_draw_pixel(ypxl2, xpxl2+1, grad(frac(yend) * xgap));
    }

    if(steep){
        for(double x = xpxl1 +1; x <= xpxl2 -1; x++){ 
            gl_draw_pixel(floor(intery), x, grad(rfpart(yend) * xgap)); //Incrementally draws the points
            gl_draw_pixel(floor(intery) + 1, x, grad(frac(yend) * xgap));
            intery = intery + gradient;
        }
    } else {
        for(double x = xpxl1 +1; x <= xpxl2 -1; x++) {
            gl_draw_pixel(x,floor(intery), grad(rfpart(yend) * xgap));
            gl_draw_pixel(x, floor(intery) + 1, grad(frac(yend) * xgap));
            intery = intery + gradient;
        }
    }
}

//Recursively floodlills the shape. Uses background color as base case because of the antialiasing of the line.
void floodFill(int x, int y, color pixColor, color background){
    if(gl_read_pixel(x, y) != background) {
        return;
    }
    gl_draw_pixel(x, y, pixColor);
    floodFill(x + 1, y, pixColor, background);
    floodFill(x - 1, y, pixColor, background);
    floodFill(x, y + 1, pixColor, background);
    floodFill(x, y - 1, pixColor, background);
}

int testInTriangle(int x0, int y0, int x1, int y1,int x2, int y2, int xCor, int yCor) {
    double area = 0.5 * (-y1 * x2 + y0*(-x1 + x2) + x0*(y1 - y2) + x1*y2);

    double s = 1/(2*area)*(y0*x2 - x0*y2 + (y2 - y0)*xCor + (x0 - x2)*yCor);
    double t = 1/(2*area)*(x0*y1 - y0*x1 + (y0 - y1)*xCor + (x1 - x0)*yCor);

    if (s>0 && t >0 && (1-s-t) > 0) return 1;
    else return 0;

}

//NOTE: make sure background is white. Draws a triangle from given x y coordinates and then fills in the triangle. Triangle may have some lighter edges due to anti-aliasing. 
void gl_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, color c){
    gl_draw_line(x2, y2, x3, y3, GL_BLACK);
    gl_draw_line(x1, y1, x2, y2, GL_BLACK);
    gl_draw_line(x3, y3, x1, y1, GL_BLACK);

    int x = 0;
    int y = 0;

    for(int i = 0; i < gl_get_width(); i++) {
        for(int j = 0; j < gl_get_height(); j++) {
            if (testInTriangle(x1, y1, x2, y2, x3, y3, i, j) == 1) {
                x = i;
                y = j;
            }
        }
    }

    floodFill(x, y, c, GL_WHITE); 
}




