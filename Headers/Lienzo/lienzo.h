#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <vfs.h>
#include "bootinfo.h"

#define LIENZO_OTYPE_WINDOW 0
#define LIENZO_OTYPE_IMAGE 1

#define LIENZO_ITYPE_PNG 0

#define LIENZO_BUFFER_SIZE(xsz,ysz,sz) ((xsz*ysz)*sz)
#define LIENZO_BUFFER_OFFSET(xsz,yoff,xoff,sz) (((yoff*xsz)+xoff)*sz)
#define LIENZO_CUTOFF(vSz,voff) ()

typedef struct rgbaS{
    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}rgba;

struct lienzoHeader{
    uint16_t objectType;
    uint16_t flags;
    uint32_t relX; //!< Relative X pos to parent
    uint32_t relY; //!< Relative Y pos to parent
    uint32_t xSz; //!< Width of object
    uint32_t ySz; //!< Height of object
};

typedef struct lienzoWindowS{
    struct lienzoHeader header;
    rgba color;
    void **children; //Children objects
    uint32_t childCount; //Number of children
    uint32_t slots; //Number of open slots
    void *buffer;
}lienzoWindow;

typedef struct lienzoImageS{
    struct lienzoHeader header;
    uint32_t *buffer; //Loaded image buffer
    uint32_t size; //Size of the buffer
}lienzoImage;

extern void initLienzo(struct bootInfo *kinf);
extern void swapLienzoBuffer();
extern lienzoWindow *lienzoCreateWindow(uint32_t xSz, uint32_t ySz, rgba color, uint16_t flags);
extern bool lienzoLoadImage(lienzoWindow *win, vFILE *image);
extern void drawLienzoWindow(lienzoWindow *win);