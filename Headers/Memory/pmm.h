#pragma once
#include <stdint.h>
#include "bootinfo.h"

#define SUBENTRY_MASK 0xFF0000

#define GET_PMM_ENTRY(p) (p/(64*PAGE_SZ))
#define GET_PMM_SUBENTRY(p) ((p/PAGE_SZ)%64)

extern void initPMM(struct bootInfo *kinf);

typedef struct{
    uint64_t start;
    uint64_t *bitmap;
    uint64_t entries;
}memRegion;

#define PMM_ALLOC_ANYMEM 0x0
#define PMM_ALLOC_HIGHMEM 0x1
#define PMM_ALLOC_LOWMEM 0x2

extern void *allocatePhys(uint16_t pages, uint8_t type);
extern void freePhys(uint64_t phys, uint64_t pages);