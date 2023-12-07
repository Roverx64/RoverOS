#pragma once

#include <stdint.h>

extern void outb(uint16 p, uint8 b);
extern uint8 inb(uint16 p);
extern void outd(uint16 p, uint32 d);
extern uint32 ind(uint16 p);