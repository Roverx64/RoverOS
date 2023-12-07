#include <stdint.h>
#include <gui.h>
#include <window.h>
#include <debug.h>

void drawFontRow(UItextout *txt, uint8 row, uint32 Xpos, uint32 Ypos){
    uint16 b = 0;
    for(int i = 0; i < 8; ++i){
        if(!(row&(0x80>>i))){Xpos += txt->Xscale; continue;}
        reDraw:
        drawPixel(txt->parent->buffer,txt->parent->common.Xr,txt->color,Xpos,Ypos);
        ++Xpos;
        ++b;
        if(b < txt->Xscale){goto reDraw;}
        b = 0;     
    }
}

void drawCharacter(UItextout *txt, UI_POS Xpos, UI_POS Ypos, UI_TEXT ch, uint8 *font){
    uint16 b = 0;
    uint32 charOffset = ((uint32)ch*5);
    for(int i = 0; i < 5; ++i){
        uint8 row = font[charOffset];
        reDraw:
        drawFontRow(txt,row,Xpos,Ypos);
        ++Ypos;
        ++b;
        if(b < txt->Yscale){goto reDraw;}
        b = 0;
        ++charOffset;
    }
}

void drawString(UIwindow *win, UItextout *txt, uint8 *font, const UI_TEXT *str){
    UI_POS Xmod = txt->common.Xp;
    UI_POS Ymod = txt->common.Yp;
    for(int i = 0; str[i] != '\0'; ++i){
        if(str[i] != '\n'){
            drawCharacter(txt,Xmod,Ymod,str[i],font);
            Xmod += (10*txt->Xscale);
        }
        else{
            Xmod = txt->common.Xp;
            Ymod += (10*txt->Yscale);
        }
    }
}

//For buffer management

void addCharacter(UItextout *output, UI_TEXT ch){
    if(output == NULL){return;}
    output->textBuffer[output->cursor] = ch;
    ++output->cursor;
}

void scroll(UItextout *output){
    if(output == NULL){return;}
    //Find breakpoint to scroll at
    int i = 0;
    for(;output->textBuffer[i] != '\n';++i){
        if(i >= output->bufferLen){return;} //No \n exists, so return
    }
    //Shift memory and set dirty
    i += 1;
    int b = 0;
    for(; b < i; ++b){
        output->textBuffer[b] = output->textBuffer[i+b];
    }
    output->cursor -= i;
    output->common.dirty = true;
    output->parent->common.dirty = true;
}

//Rendering functions

void renderText(UItextout *render){
    UI_POS Xp = render->common.Xp;
    UI_POS Yp = render->common.Yp;
    for(int i = 0; i < render->cursor; ++i){
        if(render->textBuffer[i] == '\n'){
            Xp = render->common.Xp;
            Yp += render->fYsz+render->kerning;
            continue;
        }
        drawCharacter(render,Xp,Yp,render->textBuffer[i],(uint8*)render->font);
        Xp += render->fXsz+render->kerning;
    }
}