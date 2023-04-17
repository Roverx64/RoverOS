#pragma once
#include <window.h>

extern uint8 testFont[130];
//Init functions
extern int initWindow(window *win, UI_RES Xr, UI_RES Yr);
extern int initText(window *win, text *txt, UI_COLOR color, UI_SCALE Xscale, UI_SCALE Yscale, UI_RES Xres, UI_RES Yres, UI_POS Xpos, UI_POS Ypos);
//UI Functions
extern void drawCharacter(window *win, text *txt, uint32 Xpos, uint32 Ypos, char ch, uint8 *font);
extern void drawString(window *win, text *txt, uint8 *font, const char *str);
extern void drawPixel(UI_BUFFER *buffer, UI_RES Xres, UI_COLOR color, UI_POS Xpos, UI_POS Ypos);