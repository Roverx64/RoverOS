#include <kernel.h>
#include <vmm.h>
#include <kmalloc.h>
#include <kprint.h>
#include <string.h>
#include <ramdisk.h>
#include <lienzo.h>
#include <vfs.h>
#include <libpng.h>

//RoverOS' built-in text and image renderer

void *vbuffer = NULL;
void *dbuffer = NULL; //Double buffer
uint32_t xSz;
uint32_t ySz;
uint64_t bufferLen;
uint8_t bpp; //Bytes per pixel (typically 4)

/*!
    Intilizes RoverOS' primitive text and image renderer
*/
void initLienzo(struct bootInfo *kinf){
    if(kinf->ui.framebuffer == 0x0){kerror("No framebuffer set by polarboot\n"); return;}
    bpp = 4; //Fixed for now
    //Allocate procmap space for the vmem
    vbuffer = (void*)vmallocPhys(kinf->ui.framebuffer,kinf->ui.size*bpp,VTYPE_VIMEM,VFLAG_MAKE|VFLAG_WC|VFLAG_WRITE);
    if(!vbuffer){kerror("NULL vbuffer"); return;}
    kinfo("Mapped 0x%lx->0x%lx\n",kinf->ui.framebuffer,(uint64_t)vbuffer);
    dbuffer = (void*)vmalloc(kinf->ui.size*bpp,VTYPE_VIMEM,VFLAG_MAKE|VFLAG_WC|VFLAG_WRITE);
    if(!dbuffer){kerror("NULL dbuffer"); return;}
    kinfo("Created double buffer at 0x%lx\n",(uint64_t)dbuffer);
    xSz = kinf->ui.Xres;
    ySz = kinf->ui.Yres;
    bufferLen = kinf->ui.size*bpp;
    kinfo("Video info(FB: 0x%lx|0x%lx x 0x%lx|Sz: 0x%lx)\n",kinf->ui.framebuffer,kinf->ui.Xres,kinf->ui.Yres,kinf->ui.size);
    kinfo("Intilized Lienzo renderer\n");
}

void swapLienzoBuffer(){
    memcpy(vbuffer,dbuffer,bufferLen);
}

lienzoWindow *lienzoCreateWindow(uint32_t xSz, uint32_t ySz, rgba color, uint16_t flags){
    lienzoWindow *win = (lienzoWindow*)kmalloc(sizeof(lienzoWindow));
    if(!win){return NULL;}
    win->color = color;
    win->header.xSz = xSz;
    win->header.ySz = ySz;
    win->header.flags = flags;
    win->header.objectType = LIENZO_OTYPE_WINDOW;
    win->buffer = vmalloc(LIENZO_BUFFER_SIZE(xSz,ySz,bpp),VTYPE_VIMEM,VFLAG_WRITE|VFLAG_MAKE);
    return win;
}

void clearWindow(lienzoWindow *win){
    memset(win->buffer,0x0,LIENZO_BUFFER_SIZE(win->header.xSz,win->header.ySz,bpp));
}

static inline void drawRow(void *buff, uint64_t yoff, uint64_t xoff, uint64_t xsz, lienzoWindow *win){
    size_t p = LIENZO_BUFFER_OFFSET(xsz,yoff,xoff,bpp);
    for(uint32_t x = xoff; x < xsz; ++x){
        ((uint32_t*)buff)[p+x] = 0xFF00FF;
    }
}

void drawLienzoWindow(lienzoWindow *win){
    for(uint64_t i = 0; i < win->header.ySz; ++i){
        if(i > ySz){return;}
        drawRow(dbuffer,i+win->header.relY,win->header.relX,xSz,win);
    }
}

bool lienzoLoadImage(lienzoWindow *win, vFILE *image){
    if(!checkPNG(image)){return false;}
    return true;
}