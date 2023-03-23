#pragma once
#include <stdint.h>

struct gdtPointer{
    uint16 limit;
    uint64 base;
}__attribute__((packed));

struct gdtEntry{
    uint64 ign0 : 42;
    uint8 conforming : 1;
    uint8 execute : 1;
    uint8 type : 1;
    uint8 dpl : 2;
    uint8 present : 1;
    uint8 ign1 : 5;
    uint8 lm : 1;
    uint16 ign2 : 10;
}__attribute__((packed));

#define GDT_ACCESS_PRESENT 0x1<<7
#define GDT_ACCESS_DPL(dpl) ((uint8)dpl<<5)
#define GDT_ACCESS_SYSTEM 0x0<<4
#define GDT_ACCESS_DESCRIPTOR 0x1<<4
#define GDT_ACCESS_EXECUTE 0x1<<3
#define GDT_ACCESS_DIRECTION_UP 0x1<<2
#define GDT_ACCESS_WRITE 0x1<<1

extern void initGDT();