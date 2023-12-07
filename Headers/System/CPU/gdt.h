#pragma once
#include <stdint.h>

struct gdtPointer{
    uint16 limit;
    uint64 base;
}__attribute__((packed));

#define GDT_RW (uint64)1<<43
#define GDT_CONFORMING (uint64)1<<44
#define GDT_EXECUTE (uint64)1<<43
#define GDT_SYSTEM (uint64)1<<44
#define GDT_DPL(dpl) ((uint64)dpl<<45)
#define GDT_PRESENT (uint64)1<<47
#define GDT_LM (uint64)1<<53
#define GDT_TYPE_TSS (uint64)0x9<<40
#define GDT_TSS_BASE0(addr) ((addr&0xFFFFF)<<16)
#define GDT_TSS_BASE1(addr) (((addr>>24)&0xFF)<<48)
#define GDT_TSS_BASE2(addr) ((addr>>32)&0xFFF)
extern void initGDT();
extern void writeGDTTSS(uint64 *entries, uint8 entryn, uint64 base, uint64 dpl);