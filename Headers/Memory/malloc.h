#pragma once
#include <stdint.h>

#define HEAP_FLAG_NEXT 1<<0

typedef struct heapBlock{
    uint64 magic; //Randomized per process
    size allocated; //Allocated
    size free; //Free bytes in block
    uint64 offset; //Offset to previous block
    uint16 flags;
}heapBlock;

extern void *malloc(size sz);