#pragma once
#include <stdint.h>

#define UI_SYSTEM_NAME "Graphite"
#define UI_SYSTEM_VERSION_MAJOR 0
#define UI_SYSTEM_VERSION_MINOR 9

#define UI_BUFFER uint32
#define UI_RES uint32
#define UI_POS uint32
#define UI_ID uint8
#define UI_SCALE uint16
#define UI_OBJECT uintptr
#define UI_COLOR uint32
#define UI_TEXT char

#define UI_ID_UNKNOWN 0
#define UI_ID_WINDOW 1
#define UI_ID_TEXTOUT 2

typedef struct UIcommon{
    UI_ID id;
    UI_POS Xp;
    UI_POS Yp;
    UI_RES Xr;
    UI_RES Yr;
    bool dirty; //Do we need to redraw this?
}UIcommon;

typedef struct UIwindow{
    UIcommon common;
    UI_BUFFER *buffer; //Main UI buffer that the sub buffers draw to
    UI_OBJECT *objects; //Pointer to subobjects
    uint32 objCount; //Number of objects
}UIwindow;

typedef struct UItextout{
    struct UIcommon common;
    UIwindow *parent; //Parent window
    uint32 cursor;
    UI_TEXT *textBuffer; //Pointer to the text to render 
    uint32 bufferLen; //Length of buffer allocated
    UI_SCALE Xscale; //X axis scaling
    UI_SCALE Yscale; //Y axis scaling
    uint32 kerning;
    UI_COLOR color;
    void *font;
    uint32 fXsz;
    uint32 fYsz;
    bool scroll; //Enable/Disable scrolling
}UItextout;



