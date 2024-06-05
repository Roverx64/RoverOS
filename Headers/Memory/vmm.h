#pragma once
#include <stdint.h>

#define VALLOC_FLAG_KERNEL 1<<0

typedef struct{
    uint64_t *pml4e;
    uint64_t ptr; //Bump ptr for now
    uint64_t pageHeap; //Heap space for new tables
}pageSpace;

extern uint64_t valloc(pageSpace *space, size_t bytes, uint64_t phys, uint64_t flags);
extern void vfree(pageSpace *space, uint64_t virt);