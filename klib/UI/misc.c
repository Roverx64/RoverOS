#include <stdint.h>
#include <window.h>

//I will organize these later

void drawBlock(window *win, uint32 color, uint32 Xs, uint32 Ys){
    uint32 *buff = (uint32*)win->buffer;
    uint32 pixels = Xs+(win->Xres*Ys);
    for(int i = 0; i < pixels; ++i){
        buff[i] = color;
    }
}

void drawPixel(window *win, uint32 color, uint32 Xpos, uint32 Ypos){
    uint32 offset = Xpos+(Ypos*win->Xres);
    win->buffer[offset] = color;
}