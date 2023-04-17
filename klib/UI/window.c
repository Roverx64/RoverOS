#include <stdint.h>
#include <window.h>

//Window manager for RoverOS

int initWindow(window *win, UI_RES Xr, UI_RES Yr){
    if(win == NULL){return -1;}
    win->common.Xres = Xr;
    win->common.Yres = Yr;
    //ptr->buffer = (uint8*) /*Map buffer into caller's vmap*/
    win->objects = NULL;
    return 0;
}

int initText(window *win, text *txt, UI_COLOR color, UI_SCALE Xscale, UI_SCALE Yscale, UI_RES Xres, UI_RES Yres, UI_POS Xpos, UI_POS Ypos){
    if(win == NULL || txt == NULL){return -1;}
    //Determine pos in buffer
    txt->common.Xres = Xres;
    txt->common.Yres = Yres;
    txt->common.Xpos = Xpos;
    txt->common.Ypos = Ypos;
    txt->Xscale = Xscale;
    txt->Yscale = Yscale;
    txt->color = color;
    return 0;
}