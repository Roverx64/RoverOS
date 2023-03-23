#include <stdint.h>
#include <debug.h>
#include "paging.h"
#include "mmap.h"
#include "pmm.h"
#include "bootinfo.h"

//For managing physical pages

kblock *start = NULL;

void initPMM(struct bootInfo *kinf){
    kdebug(DINFO,"Initilizing pmm with 0x%llx MB of total ram\n",(uint64)(kinf->mem.totalMem/1048576));
    mmapEntry *mmap = (mmapEntry*)kinf->mem.ptr;
    uint64 usableRam = 0x0;
    for(int i = 0; i < kinf->mem.entries; ++i){
        switch(mmap[i].type){
            case MMAP_TYPE_FREE:
            usableRam += mmap[i].pages*PAGE_SZ;
            break;
        }
    }
    kdebug(DINFO,"Found 0x%llx MB of usable ram\n",(uint64)(kinf->mem.usableMem/1048576));
}

//Returns a pointer to a free phys page
void *palloc(uint64 sz, bool align){

}

//Frees a phys page
void pfree(void *ptr){
    
}