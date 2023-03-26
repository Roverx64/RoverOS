#include <stdint.h>
#include <debug.h>
#include "paging.h"
#include "mmap.h"
#include "pmm.h"
#include "bootinfo.h"
#include "palloc.h"
#include <string.h>

uint32 pmmEntries;
uint64 *pagemap;

void setPMMBit(uint64 phys, bool free){
    phys = KALIGN(phys); //Get pagemap entry
    uint32 pgentry = GET_PMM_ENTRY(phys);
    uint32 subentry = GET_PMM_SUBENTRY(phys);
    if(free){pagemap[pgentry] ^= (0x1<<subentry);}
    else{pagemap[pgentry] |= (0x1<<subentry);}
}

void initMmapEntry(mmapEntry *entry){
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
    for(int i = 0;i < pages; ++i){
        setPMMBit(entry->phys+(i*PAGE_SZ),set);
    }
}

//Only manages free memory
void initPMM(struct bootInfo *kinf){
    kdebug(DINFO,"Initilizing pmm with 0x%llx MB of total ram\n",(uint64)(kinf->mem.totalMem/1048576));
    mmapEntry *mmap = (mmapEntry*)kinf->mem.ptr;
    pmmEntries = (kinf->mem.totalMem/PAGE_SZ)/64;
    uint64 sz = pmmEntries*sizeof(uint64);
    //Find free spot for pagemap
    for(int i = 0; i < kinf->mem.entries; ++i){
        if(mmap[i].type == MMAP_TYPE_FREE && mmap[i].bytes >= sz){
            pagemap = (uint64*)mmap[i].phys;
            break;
        }
    }
    memset(pagemap,sz,0x0);
    //Initilize all of mem
    bool set = false;
    for(uint32 i = 0; i < kinf->mem.entries; ++i){
        initMmapEntry(&mmap[i]);
    }
    kdebug(DINFO,"Initilized PMM\n");
}

//Returns a pointer to a phys page
void *palloc(uint64 sz){
    uint64 test = 0xFFFF;
    uint16 shift = 0x0;
    uint16 word = 0x0;
    uint16 pos = 0x0;
    int i = 0;
    for(; i < pmmEntries; ++i){
        if(pagemap[i] == 0xFFFFFFFFFFFFFFFF){continue;} //All pages allocated
        //Shift until free bit is found
        while(pagemap[i]&(test<<shift) == test){shift+=16;}
        word = (pagemap[i]&(test<<shift))>>shift;
        //Find free bit
        int bit = 0;
        for(;bit < 16;++bit){
            if(word&(1<<bit) == 0x0){break;}
        }
        //Determine final pos
        pos = (shift-16)+bit;
        break;
    }
    //Do math and get phys addr
    uint64 phys = ((i*64)*PAGE_SZ)+(PAGE_SZ*pos);
    setPMMBit(phys,false);
    return (void*)phys;
}

//Frees a phys page
void pfree(void *ptr){
    if(IS_ALIGNED(ptr)){kdebug(DERROR,"Unaligned ptr 0x%llx",(uint64)ptr); return;} //Pointer is not valid
    setPMMBit((uint64)ptr,true);
}