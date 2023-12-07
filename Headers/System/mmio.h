#pragma once

#include <stdint.h>

#define MMIO_AQWORD 0x4 //Atomic qword (Ignored for writes)
#define MMIO_QWORD 0x0
#define MMIO_DWORD 0x1
#define MMIO_WORD 0x2
#define MMIO_BYTE 0x3

extern void writeMMIO(void *bar, uint64 offset, uint64 v, uint64 sz);
extern uint64 readMMIO(void *bar, uint64 offset, uint64 sz);

#define rlmmio(bar,reg,dest) asm volatile("add %1, %2; movq %2, %%rdx; movq (%%rdx), %0":"=r" ((uint64)dest):"r" ((uint64)bar), "r" ((uint64)reg):"rdx") //Read u64 mmio
#define rdmmio(bar,reg,dest) asm volatile("add %1, %2; movq %2, %%rdx; mov (%%rdx), %0":"=r" ((uint32)dest):"r" ((uint64)bar), "r" ((uint64)reg):"rdx") //Read u32 mmio

//#define walmmio(addr,val) asm volatile("movq %1, %%rdx; movq %0, (%%rdx)":::"rdx") //Write atomic u64 mmio
//#define wlmmio(addr,val) asm volatile("movq %1, %%rdx; movq %0, rcx; and $0xFFFFFFFF, rcx; \n\t" 
//                                      "movq rcx, (%%rdx); movq %0, rcx; shl 32, rcx; movd ecx, 4(%%rdx)":::) //Write u64 mmio

#define wlmmio(bar,reg,val) asm volatile("add %1, %2;movq %2, %%rdx; movq %0, (%%rdx)"::"r" (val), "r" (bar), "r"(reg):"rdx") //Write u64 mmio
#define wdmmio(bar,reg,val) asm volatile("add %1, %2;movq %2, %%rdx; mov %0, (%%rdx)"::"r" (val), "r" (bar), "r"(reg):"rdx") //Write u32 mmio