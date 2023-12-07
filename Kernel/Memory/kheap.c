#include <stdint.h>
#include <kernel.h>
#include <string.h>
#include "debug.h"
#include "paging.h"
#include "kheap.h"
#include "pmm.h"

kheapBlock *startBlock = NULL;

void initKheap(void *heapVBase, uint64 heapSize){
    startBlock = (kheapBlock*)heapVBase;
    memset(startBlock,0x0,sizeof(kheapBlock));
    startBlock->magic = KHEAP_MAGIC;
    startBlock->allocated = 0x0;
    startBlock->free = heapSize-sizeof(kheapBlock);
    kdebug(DINFO,"Initilized heap at 0x%lx\n",(uint64)heapVBase);
}

void extendHeap(kheapBlock *prev){
    void *pg = allocatePhys(1,PMM_ALLOC_ANYMEM);
    if(pg == NULL){kpanic("Failed to extend kernel heap",0x0);}
    kheapBlock *ex = (kheapBlock*)pg;
    ex->magic = KHEAP_MAGIC;
    ex->allocated = 0;
    ex->boundary = true;
    ex->free = PAGE_SZ-sizeof(kheapBlock);
    ex->prev = prev;
    ex->next = NULL;
    kdebug(DINFO,"Extended kheap\n");
    return;
}

//May cause a performance issue in the future with allocations after a shrink
void shrinkHeap(kheapBlock *bound){
    ((kheapBlock*)bound->prev)->next = NULL;
    kdebug(DINFO,"Shrunk kheap\n");
    freePhys((uint64_t)bound,1);
}

void *halloc(size sz){
    kheapBlock *block = startBlock;
    size csz = sz+sizeof(kheapBlock);
    for(;block->free < csz;){
        if(block->next == NULL){extendHeap(block);}
        block = (kheapBlock*)block->next;
    }
    kheapBlock *nblock = (kheapBlock*)((uint64)block+block->allocated+sizeof(kheapBlock));
    nblock->magic = KHEAP_MAGIC;
    nblock->free = block->free-csz;
    nblock->allocated = sz;
    nblock->prev = (void*)block;
    nblock->next = block->next;
    block->free = 0x0;
    block->next = (void*)nblock;
    if(nblock->next != NULL){((kheapBlock*)nblock->next)->prev = (void*)nblock;}
    return (void*)((uint64)nblock+sizeof(kheapBlock));
}

void hfree(void *ptr){
    kheapBlock *block = (kheapBlock*)((uint64)ptr-sizeof(kheapBlock));
    if(block->magic != KHEAP_MAGIC){kpanic("Invalid ptr",(uint64)ptr);}
    ((kheapBlock*)block->prev)->next = block->next;
    ((kheapBlock*)block->prev)->free += block->free+block->allocated+sizeof(kheapBlock);
    ((kheapBlock*)block->next)->prev = block->prev;
    block->magic = 0x0;
    if(block->boundary == true){shrinkHeap(block);}
    return;
}