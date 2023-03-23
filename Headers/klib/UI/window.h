#pragma once
#include <stdint.h>

#define WINDOW_SYSTEM_NAME "Graphite"
#define WINDOW_SYSTEM_VERSION_MAJOR 0
#define WINDOW_SYSTEM_VERSION_MINOR 2

#define WINDOW_BUFFER uint32
#define WINDOW_XRES uint32
#define WINDOW_YRES uint32
#define WINDOW_SCALE uint16
#define WINDOW_OBJECT uintptr

#define GUI_BUFFER_OFFSET()

typedef struct window{
    WINDOW_BUFFER *buffer;
    WINDOW_XRES Xres;
    WINDOW_YRES Yres;
    WINDOW_SCALE Xscale;
    WINDOW_SCALE Yscale;
    WINDOW_OBJECT *objects;
}window;

#define WINDOW_PTR window

extern void drawBlock(window *win,uint32 color,uint32 Xs, uint32 Ys);