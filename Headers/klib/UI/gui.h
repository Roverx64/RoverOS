#pragma once
#include <window.h>

//Init functions
extern int initWindow(UIwindow *win, UI_RES Xr, UI_RES Yr);
extern int initText(UIwindow *win, UItextout *txt, UI_COLOR color, UI_SCALE Xscale, UI_SCALE Yscale, UI_RES Xres, UI_RES Yres, UI_POS Xpos, UI_POS Ypos);
//UI Functions
extern void drawCharacter(UItextout *txt, UI_POS Xpos, UI_POS Ypos, UI_TEXT ch, uint8 *font);
extern void drawString(UIwindow *win, UItextout *txt, uint8 *font, const UI_TEXT *str);
extern void drawPixel(UI_BUFFER *buffer, UI_RES Xres, UI_COLOR color, UI_POS Xpos, UI_POS Ypos);
extern void addCharacter(UItextout *output, UI_TEXT ch);
extern void scroll(UItextout *output);