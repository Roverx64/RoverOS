#pragma once
#include <stdint.h>

#define UI_SYSTEM_NAME "Graphite"
#define UI_SYSTEM_VERSION_MAJOR 0
#define UI_SYSTEM_VERSION_MINOR 4

#define UI_BUFFER uint32
#define UI_RES uint32
#define UI_POS uint32
#define UI_ID uint8
#define UI_SCALE uint16
#define UI_OBJECT uintptr
#define UI_COLOR uint32
#define UI_TEXT char

#define UI_BUFFER_OFFSET(buff,Xres,Xpos,Ypos) ((UI_BUFFER*)((uint32)buff+((Ypos*Xres)+Xpos)))

struct objectCommon{
    UI_ID id;
    UI_RES Xres;
    UI_RES Yres;
    UI_POS Xpos; //Always 0 for a window
    UI_POS Ypos; //Always 0 for a window
};

typedef struct window{
    struct objectCommon common;
    UI_BUFFER *buffer;
    UI_OBJECT *objects;
}window;

typedef struct text{
    struct objectCommon common;
    UI_SCALE Xscale;
    UI_SCALE Yscale;
    UI_TEXT *text;
    UI_COLOR color;
}text;