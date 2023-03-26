#include <stdint.h>
#include <debug.h>
#include "paging.h"
#include "mmap.h"
#include "pmm.h"
#include "bootinfo.h"

//For managing physical pages

kblock blockStart;

void initPMM(struct bootInfo *kinf){
    kdebug(DINFO,"Initilizing pmm with 0x%llx MB of total ram\n",(uint64)(kinf->mem.totalMem/1048576));
    mmapEntry *mmap = (mmapEntry*)kinf->mem.ptr;
    uint64 freeRam = 0x0;
    kblock *prev = NULL;
    kblock *current = &blockStart;
    for(int i = 0; i < kinf->mem.entries; ++i){
        switch(mmap[i].type){
            case MMAP_TYPE_FREE:
            current->ptr = mmap[i].phys;
            current->pages = mmap[i].bytes/PAGE_SZ;
            current->free = current->pages;
            current->prev = prev;
            prev = current;
            freeRam += mmap[i].bytes;
            break;
        }
    }
    kdebug(DINFO,"Found 0x%llx MB of free ram\n",(uint64)(freeRam/1048576));
}

//Returns a pointer to a free phys page
void *palloc(uint64 sz, bool align){

}

//Frees a phys page
void pfree(void *ptr){
    
}