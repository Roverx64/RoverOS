#include <stdint.h>
#include <com.h>
#include <debug.h>
#include <stdarg.h>
#include <gui.h>
#include "kheap.h"
#define CAP_MASK 0xDF //& this to capitalize a 8 bit char

char ch = 'A';
bool uiDebug = false;
UIwindow debugWindow;
UItextout output;
UI_OBJECT *obj[1];


void writeHex(uint64 num,uint16 bytes,bool caps){
    uint8 nibble = 0x0;
    bool leading = true;
    uint16 bits = (bytes*8);
    uint8 ch = 0x0;
    uint64 mask = (uint64)0xF000000000000000;
    for(int i = 0;i < bits;i+=4){
        nibble = (uint8)((uint64)(num&mask)>>(bits-(i+4)));
        mask = mask>>4;
        if(leading && nibble == 0x0){continue;}
        leading = false;
        if(nibble >= 0xA){
            ch = nibble+87;
            if(caps){ch &= CAP_MASK;}
        }
        else{
            ch = nibble|0x30;
        }
        if(uiDebug){addCharacter(&output,(UI_TEXT)ch);}
        writeByte(KDEBUG_PORT,ch);
        ch = 0x0;
    }
    if(leading){writeByte(KDEBUG_PORT,'0'); /*addCharacter(&output,(UI_TEXT)ch);*/}
}

void kprintChar(char ch){
    if(uiDebug){addCharacter(&output,(UI_TEXT)ch);}
    writeByte(KDEBUG_PORT,ch);
}

void kprintString(const char *str){
    for(int i = 0; str[i] != '\0';++i){
        kprintChar(str[i]);
    }
}

void kdebug(const char *func, uint8 level, const char *str, ...){
    uint8 sizeModifier = 0; //u8 1, u32 4, u64 8
    va_list args;
    va_start(args,str);
    char dchar = 'g';
    bool caps = false;
    switch(level){
        case DLEVEL_INFO:
        dchar = 'i';
        break;
        case DLEVEL_WARN:
        dchar = 'w';
        break;
        case DLEVEL_ERROR:
        dchar = 'e';
        break;
        case DLEVEL_FATAL:
        dchar = 'f';
        break;
        case DLEVEL_NONE:
        goto skip;
        break;
    }
    kprintString(func);
    kprintChar('[');
    kprintChar(dchar);
    kprintChar(']');
    kprintChar(':');
    skip:
    for(int i = 0; str[i] != '\0'; ++i){
        if(str[i] != '%'){
            kprintChar(str[i]);
            continue;
        }
        loop:
        ++i;
        switch(str[i]){
            case 'i':
            break;
            case 'X':
            caps = true;
            writeHex(va_arg(args,uint64),sizeof(uint64),caps);
            break;
            case 'x':
            writeHex(va_arg(args,uint64),sizeof(uint64),caps);
            break;
            case 'c':
            kprintChar(va_arg(args,int));
            break;
            case 'h':
            sizeModifier = 1;
            goto loop;
            break;
            case 'l':
            sizeModifier += 4;
            goto loop;
            break;
            case 's':
            kprintString(va_arg(args,char*));
            break;
            default:
            break;
        }
        sizeModifier = 0;
    }
    //if(uiDebug){renderText(&output);}
}

//Setup UI debugging
void initUIDebug(void *buffer, uint32 Xr, uint32 Yr){
    //Manually setup the window for now
    debugWindow.buffer = (UI_BUFFER*)buffer;
    debugWindow.common.id = UI_ID_WINDOW;
    debugWindow.common.Xp = 0;
    debugWindow.common.Yp = 0;
    debugWindow.common.Xr = Xr;
    debugWindow.common.Yr = Yr;
    debugWindow.common.dirty = false;
    debugWindow.objects = &obj;
    debugWindow.objCount = 1;
    obj[0] = (UI_OBJECT*)&output;
    //Setup output
    output.common.id = UI_ID_TEXTOUT;
    output.common.Xp = 0;
    output.common.Yp = 0;
    output.common.Xr = Xr;
    output.common.Yr = Yr;
    output.cursor = 0;
    output.common.dirty = false;
    output.parent = &debugWindow;
    output.textBuffer = (UI_TEXT*)halloc(sizeof(UI_TEXT)*6000);
    output.bufferLen = 1000;
    output.Xscale = 1;
    output.Yscale = 1;
    output.scroll = true;
    //output.font = &testFont;
    output.kerning = 2;
    output.fXsz = 8;
    output.fYsz = 8;
    output.color = 0x00FFFFFF;
    uiDebug = true;
}