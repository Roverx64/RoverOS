#include <stdint.h>
#include <window.h>

//I will organize these later

void drawPixel(UI_BUFFER *buffer, UI_RES Xres, UI_COLOR color, UI_POS Xpos, UI_POS Ypos){
    uint32 offset = Xpos+(Ypos*Xres);
    buffer[offset] = color;
}