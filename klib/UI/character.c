#include <stdint.h>
#include <window.h>
#include <debug.h>

void drawFontRow(window *win, uint8 row, uint32 Xpos, uint32 Ypos){
    uint16 b = 0;
    for(int i = 0; i < 8; ++i){
        if(!(row&(0x80>>i))){Xpos += win->Xscale; continue;}
        reDraw:
        drawPixel(win,0xFFFFFF,Xpos,Ypos);
        ++Xpos;
        ++b;
        if(b < win->Xscale){goto reDraw;}
        b = 0;     
    }
}

void drawCharacter(window *win, uint32 Xpos, uint32 Ypos, char ch, uint8 *font){
    uint16 b = 0;
    uint16 charOffset = (ch-0x41)*5;
    for(int i = 0; i < 5; ++i){
        uint8 row = font[charOffset];
        reDraw:
        drawFontRow(win,row,Xpos,Ypos);
        ++Ypos;
        ++b;
        if(b < win->Yscale){goto reDraw;}
        b = 0;
        ++charOffset;
    }
}

void drawString(window *win, uint32 Xpos, uint32 Ypos, uint8 *font, const char *str){
    uint32 Xmod = Xpos;
    for(int i = 0; str[i] != '\0'; ++i){
        if(str[i] != '\n'){
            drawCharacter(win,Xmod,Ypos,str[i],font);
            Xmod += (10*win->Xscale);
        }
        else{
            Xmod = Xpos;
            Ypos += (10*win->Yscale);
        }
    }
}