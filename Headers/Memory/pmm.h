#pragma once
#include <stdint.h>
#include "bootinfo.h"

//For record keeping
typedef struct{
    uint64 ptr; //Pointer to a phys addr
    uint16 pages; //Pages allocated
    uint16 free; //Pages free
    struct kblock *prev;
    struct kblock *next;
}kblock;

extern void initPMM(struct bootInfo *kinf);