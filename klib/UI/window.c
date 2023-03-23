#include <stdint.h>
#include <window.h>

//Window manager for RoverOS

int createWindow(WINDOW_PTR *ptr, WINDOW_XRES Xr, WINDOW_YRES Yr){
    if(ptr == NULL){return -1;}
    ptr->Xres = Xr;
    ptr->Yres = Yr;
    //ptr->buffer = (uint8*) /*Map buffer into caller's pmap*/
    ptr->objects = NULL;
    return 0;
}