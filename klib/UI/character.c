#include <stdint.h>
#include <gui.h>
#include <window.h>
#include <debug.h>

void drawFontRow(window *win, text *txt, uint8 row, uint32 Xpos, uint32 Ypos){
    uint16 b = 0;
    for(int i = 0; i < 8; ++i){
        if(!(row&(0x80>>i))){Xpos += txt->Xscale; continue;}
        reDraw:
        drawPixel(win->buffer,win->common.Xres,txt->color,Xpos,Ypos);
        ++Xpos;
        ++b;
        if(b < txt->Xscale){goto reDraw;}
        b = 0;     
    }
}

void drawCharacter(window *win, text *txt, uint32 Xpos, uint32 Ypos, char ch, uint8 *font){
    uint16 b = 0;
    uint16 charOffset = (ch-0x41)*5;
    for(int i = 0; i < 5; ++i){
        uint8 row = font[charOffset];
        reDraw:
        drawFontRow(win,txt,row,Xpos,Ypos);
        ++Ypos;
        ++b;
        if(b < txt->Yscale){goto reDraw;}
        b = 0;
        ++charOffset;
    }
}

void drawString(window *win, text *txt, uint8 *font, const char *str){
    uint32 Xmod = txt->common.Xpos;
    uint32 Ymod = txt->common.Ypos;
    for(int i = 0; str[i] != '\0'; ++i){
        if(str[i] != '\n'){
            drawCharacter(win,txt,Xmod,Ymod,str[i],font);
            Xmod += (10*txt->Xscale);
        }
        else{
            Xmod = txt->common.Xpos;
            Ymod += (10*txt->Yscale);
        }
    }
}