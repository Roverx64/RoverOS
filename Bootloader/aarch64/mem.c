#include <stdint.h>
#include <mmap.h>
#include <paging.h>

uint64_t base;
uint64_t max;
uint64_t cursor = 0;
mmapEntry *mmap;
mmapEntry *amap; //NULL until the device tree is fully parsed
uint64_t mmapEntries = 0;
uint64_t allocEntries = 0;
uint64_t ldAddr;
uint64_t ldSize;
uint64_t *pageBase = NULL;

void addMmapEntry(uint64_t phys, uint64_t virt, size_t len, uint64_t type){
    mmap[cursor].offset = 0;
    mmap[cursor].type = type;
    mmap[cursor].phys = phys;
    mmap[cursor].virt = virt;
    mmap[cursor].bytes = len;
    ++cursor;
}

void addAllocation(uint64_t phys, uint64_t virt, size_t sz, uint64_t type){
    amap[cursor].phys = phys;
    amap[cursor].virt = virt;
    amap[cursor].bytes = sz;
    amap[cursor].type = type;
    ++cursor;
    ++allocEntries;
}

void initAllocmmap(){
    mmapEntries = cursor;
    amap = (mmapEntry*)(base+(cursor*sizeof(mmapEntry)));
    amap[0].phys = base;
    amap[0].virt = base;
    amap[0].bytes = max;
    amap[0].type = MMAP_TYPE_BOOT;
    allocEntries = 1;
    cursor = 1;
}

void initMem(uint64_t mem, uint64_t sz, uint64_t loaderAddr, uint64_t loaderSize){
    base = mem;
    max = sz;
    mmap = (mmapEntry*)base;
    ldAddr = loaderAddr;
    ldSize = loaderSize;
}

//Allocates from the mmap and may map to a desired va
//Rounds up to nearest 4K aligned size
//if virt is 1, then identity mapping is used
void *allocMem(size_t sz, uint64_t type, uint64_t virt, bool map){
    if(sz%0x1000 != 0){sz = (sz+0x1000)&(~0xFFF);}
    for(uint64_t i = 0; i < mmapEntries; ++i){
        if(mmap[i].type != MMAP_TYPE_FREE){continue;}
        if(mmap[i].offset+sz >= mmap[i].bytes){continue;}
        uint64_t addr = mmap[i].phys+sz;
        mmap[i].offset += sz;
        uint64_t va = ((map == 1) && (virt != 1)) ? virt : addr;
        addAllocation(addr,va,sz,type);
        return (void*)addr;
    }
    return NULL;
}

//Paging stuff

void setPageMap(uint64_t val){
    asm volatile("msr TTBR0_EL1, %0":"r"(val):);
}

void mapPage(uint64_t phys, uint64_t virt, uint8_t flags){

}

void initPaging(){
    //Allocate base table
    pageBase = (uint64_t*)allocMem(sizeof(uint64_t)*512,MMAP_TYPE_PAGE,1,false);
    setPageMap((uint64_t)pageBase);
    
}