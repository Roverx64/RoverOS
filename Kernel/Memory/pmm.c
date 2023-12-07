#include <stdint.h>
#include <debug.h>
#include "paging.h"
#include "mmap.h"
#include "pmm.h"
#include "bootinfo.h"
#include "kheap.h"
#include <string.h>

memRegion lowMem;
memRegion highMem;

void markPages(memRegion *mem, uint64_t ptr, uint64_t pages, bool free){
    //if(!IS_ALIGNED(ptr)){return;}
    ptr = ptr-mem->start;
    uint64_t index = GET_PMM_ENTRY(ptr);
    uint64_t set = ((0xFFFFFFFFFFFFFFFF>>(64-pages))<<GET_PMM_SUBENTRY(ptr));
    if(!free){mem->bitmap[index] |= set; return;}
    if((mem->bitmap[index]&set) != set){return;}
    mem->bitmap[index] ^= set;
}

void initMmapEntry(memRegion *mem, mmapEntry *entry){
    if(entry->magic != MMAP_MAGIC){kdebug(DERROR,"Invalid MMAP magic\n"); return;}
    bool set = true;
    switch(entry->type){
        case MMAP_TYPE_UEFI:
        case MMAP_TYPE_VESA:
        case MMAP_TYPE_MMIO:
        case MMAP_TYPE_ACPI:
        case MMAP_TYPE_RESV:
        case MMAP_TYPE_UNUSABLE:
        set = false;
        break;
    }
    uint32 pages = entry->bytes/PAGE_SZ;
    markPages(mem,entry->phys,(entry->bytes/PAGE_SZ),set);
}

//Only manages free memory
void initPMM(struct bootInfo *kinf){
    kdebug(DINFO,"Initilizing pmm with 0x%lx MB (0x%lx GB) of total ram\n",(uint64)(kinf->mem.totalMem/0x200000),(uint64)(kinf->mem.totalMem/0x40000000));
    uint64_t lsz = kinf->mem.totalMem/0x40000000;
    uint64_t hsz = 0;
    if((kinf->mem.totalMem/0x40000000) > 4){
        lsz = 4;
        hsz = (kinf->mem.totalMem/0x40000000)-4;
        lowMem.entries = ((lsz*0x40000000)/PAGE_SZ)/64;
        highMem.entries = ((hsz*0x40000000)/PAGE_SZ)/64;
    }
    kdebug(DINFO,"%lxGB low | %lxGB high\n",lsz,hsz);
    lowMem.start = 0x0;
    highMem.start = ((uint64_t)0x40000000*(uint64_t)4);
    void *ptr = halloc((highMem.entries+lowMem.entries)*sizeof(uint64_t));
    kdebug(DINFO,"Allocated bitmap to 0x%lx\n",(uint64_t)ptr);
    memset(ptr,0x0,(highMem.entries+lowMem.entries)*sizeof(uint64_t));
    lowMem.bitmap = (uint64*)ptr;
    if(hsz != 0){highMem.bitmap = (uint64_t*)((uint64_t)ptr+(sizeof(uint64_t)*lowMem.entries));}
    else{highMem.bitmap = NULL;}
    mmapEntry *entries = (mmapEntry*)kinf->mem.ptr;
    memRegion *mem = NULL;
    for(uint64_t i = 0; i < kinf->mem.entries; ++i){
        if(entries[i].phys >= highMem.start){mem = &highMem;}
        else{mem = &lowMem;}
        initMmapEntry(mem,&entries[i]);
    }
    markPages(&lowMem,0x0,1,false);
    kdebug(DINFO,"Initilized PMM\n");
}

//NOTE: Might add an option for memory below 1MB for SMP trampoline
void *allocatePhys(uint16_t pages, uint8_t type){
    if((pages == 0) || (pages > 64)){return NULL;}
    uint64_t i = 0;
    int b = 0;
    bool lpDone = false;
    uint64_t test = (0xFFFFFFFFFFFFFFFF>>(64-pages));
    memRegion *mem = &lowMem;
    if(type == PMM_ALLOC_HIGHMEM){mem = &highMem;}
    if(type == PMM_ALLOC_LOWMEM){mem = &lowMem;}
    lp:
    for(; i < mem->entries; ++i){
        if(mem->bitmap[i] == 0xFFFFFFFFFFFFFFFF){continue;}
        //Test for contiguous section of pages
        for(; b < (64-pages); ++b){
            if((mem->bitmap[i]&(test<<b)) == 0x0){goto ret;}
        }
        break;
    }
    if((highMem.bitmap != NULL) && (type == PMM_ALLOC_ANYMEM) && (lpDone != true)){
        mem = &highMem;
        lpDone = true;
        i = 0;
        goto lp;
    }
    return NULL;
    ret:
    //Determine address
    uint64_t addr = mem->start+(((i*64)+b)*PAGE_SZ);
    //Mark pages as used
    markPages(mem,addr,pages,false);
    return (void*)addr;
}

void freePhys(uint64_t phys, uint64_t pages){
    memRegion *mem = NULL;
    if(phys >= highMem.start){mem = &highMem;}
    else{mem = &lowMem;}
    markPages(mem,phys,pages,true);
}