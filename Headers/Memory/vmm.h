#pragma once
#include <stdint.h>

typedef struct{
    uint64 *pml4e;
    uint64 *heapBase;
    uint64 heapSize;
    uint64 heapUsed;
}pageSpace;

extern uint64 valloc();