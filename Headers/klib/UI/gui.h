#pragma once
#include <window.h>

extern uint8 testFont[70];
extern void printCharacter(window *win, uint32 Xpos, uint32 Ypos, char ch, uint8 *font);
extern void drawString(window *win, uint32 Xpos, uint32 Ypos, uint8 *font, const char *str);
extern void drawPixel(window *win, uint32 color, uint32 Xpos, uint32 Ypos);