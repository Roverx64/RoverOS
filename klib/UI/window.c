#include <stdint.h>
#include <window.h>

//Window manager for RoverOS

int initWindow(UIwindow *win, UI_RES Xr, UI_RES Yr){
    if(win == NULL){return -1;}
    win->common.Xr = Xr;
    win->common.Yr = Yr;
    win->buffer = NULL;
    win->objects = NULL;
    return 0;
}

int initText(UIwindow *win, UItextout *txt, UI_COLOR color, UI_SCALE Xscale, UI_SCALE Yscale, UI_RES Xres, UI_RES Yres, UI_POS Xpos, UI_POS Ypos){
    if(win == NULL || txt == NULL){return -1;}
    //Determine pos in buffer
    txt->common.Xr = Xres;
    txt->common.Yr = Yres;
    txt->common.Xp = Xpos;
    txt->common.Yp = Ypos;
    txt->Xscale = Xscale;
    txt->Yscale = Yscale;
    txt->color = color;
    return 0;
}

//Re-render a window
void render(UIwindow *window){
    if(!window->common.dirty){return;} //Nothing to do
    //Redraw sub-objects

}