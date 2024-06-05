#include <kmalloc.h>
#include <mutex.h>
#include <stdint.h>
#include <kernel.h>
#include <string.h>
#include <task.h>
#include "debug.h"
#include "paging.h"
#include "pmm.h"
#include "vmm.h"

extern pageSpace kspace;
extern uint64_t linkerHeapRegion;

kheapBlock *startBlock = NULL;
mutex_t kmallocLock = 0;
uint64_t kheapBaseVirt; //Base virtual address
uint64_t allocatedRegions = 0; //Used for calculation address offset

extern void initSLAB(struct bootInfo *kinf);

void initKmalloc(struct bootInfo *kinf){
    //Allocate temporary page heap
    uint64_t heapPhys = allocPhys(0,MEMORY_TYPE_HGH,0);
    //Ensure it is writeable
    kdebug(DINFO,"Temp heap at 0x%lx\n",heapPhys);
    mapPages(&kspace,heapPhys,heapPhys,PAGE_FLAG_WRITE,ORDER_MIN_BLOCK_SIZE/PAGE_SZ);
    kspace.pageHeap = heapPhys;
    kspace.ptr = 0x0;
    //Allocate kheap space
    uint64_t phys = allocPhys(0,MEMORY_TYPE_ANY,0);
    kheapBaseVirt = (uint64_t)&linkerHeapRegion;
    kdebug(DINFO,"Mapping heap 0x%lx to 0x%lx\n",phys,kheapBaseVirt);
    mapPages(&kspace,phys,KHEAP_CALC_OFFSET(kheapBaseVirt,allocatedRegions),PAGE_FLAG_WRITE|PAGE_FLAG_MAKE,ORDER_MIN_BLOCK_SIZE/PAGE_SZ);
    startBlock = (kheapBlock*)KHEAP_CALC_OFFSET(kheapBaseVirt,allocatedRegions);
    ++allocatedRegions;
    memset(startBlock,0x0,sizeof(kheapBlock));
    startBlock->magic = KHEAP_MAGIC;
    startBlock->allocated = 0x0;
    startBlock->free = ORDER_BLOCK_SIZE(0)-(sizeof(kheapBlock)+sizeof(uint64_t));
    uint64_t *ptr = (uint64_t*)KHEAP_REDZONE_OFFSET(startBlock);
    *ptr = KHEAP_REDZONE_MAGIC;
    kdebug(DINFO,"Initilized heap at 0x%lx\n",(uint64_t)startBlock);
    initSLAB(kinf);
}

static kheapBlock *extendHeap(){
    uint64_t phys = allocPhys(0,MEMORY_TYPE_ANY,0);
    kheapBlock *nblock = (kheapBlock*)valloc(&kspace,ORDER_BLOCK_SIZE(0),phys,PAGE_FLAG_WRITE);
    if(nblock == NULL){return NULL;}
    kdebug(DINFO,"Extended kheap with new mem at 0x%lx\n",(uint64_t)nblock);
    nblock->allocated = 0;
    nblock->free = ORDER_BLOCK_SIZE(0)-(sizeof(kheapBlock)+sizeof(uint64_t));
    nblock->chunkBorder = true;
    nblock->prev = NULL;
    nblock->next = NULL;
    uint64_t *ptr = (uint64_t*)KHEAP_REDZONE_OFFSET(nblock);
    *ptr = KHEAP_REDZONE_MAGIC;
    return nblock;
}

static void shrinkHeap(kheapBlock *block){
    if(block->chunkBorder != true){return;}
    ((kheapBlock*)block->prev)->next = NULL;
    uint64_t phys = virtToPhys(&kspace,(uint64_t)block);
    vfree(&kspace,(uint64_t)block);
    freePhys(phys);
}

static inline void checkRedzone(kheapBlock *block){
    uint64_t *rdz = (uint64_t*)KHEAP_REDZONE_OFFSET(block);
    if(*rdz != KHEAP_REDZONE_MAGIC){kpanic("Corrupted redzone",(uint64_t)block);}
}

static inline void checkMagic(kheapBlock *block){
    if(block->magic != KHEAP_MAGIC){kpanic("Invalid magic",(uint64_t)block);}
}


void *kmallocInt(size_t sz, bool align){
    //Align allocates on a 4KB boundary
    kheapBlock *block = startBlock;
    kheapBlock *nblock = NULL;
    bool split = false;
    size_t csz = sz+sizeof(kheapBlock)+sizeof(uint64_t);
    if(align){csz += 0x1000;}
    for(;;){
        checkMagic(block);
        if(block->free >= csz){split = true; break;}
        if(block->next == NULL){break;}
        block = (kheapBlock*)block->next;
    }
    checkRedzone(block);
    if(align && split){
        uint64_t ptr = KHEAP_NEWBLK_LOC(block)+0x1000;
        uint64_t extra = (ptr&0xFFF)-sizeof(kheapBlock);
        ptr &= 0xFFFFFFFFFFFFF000;
        nblock = (kheapBlock*)(ptr-sizeof(kheapBlock));
        nblock->allocated = sz;
        nblock->free = (block->free-csz)-extra;
        nblock->chunkBorder = false;
        nblock->prev = (void*)block;
        nblock->next = block->next;
        goto end;
    }
    if(split){
        nblock = (kheapBlock*)KHEAP_NEWBLK_LOC(block);
        nblock->allocated = sz;
        nblock->free = block->free-(sizeof(uint64_t)+sz+sizeof(kheapBlock));
        nblock->chunkBorder = false;
        nblock->prev = (void*)block;
        nblock->next = block->next;
        block->next = (void*)nblock;
        if(nblock->next != NULL){((kheapBlock*)nblock->next)->prev = (void*)nblock;}
        goto end;
    }
    //Heap needs more space
    nblock = extendHeap();
    nblock->allocated = sz;
    nblock->free -= sz;
    block->next = (void*)nblock;
    nblock->prev = (void*)block;
    nblock->chunkBorder = true;
    end:
    uint64_t *rdz = (uint64_t*)KHEAP_REDZONE_OFFSET(nblock);
    *rdz = KHEAP_REDZONE_MAGIC;
    nblock->magic = KHEAP_MAGIC;
    return (void*)((uint64_t)nblock+sizeof(kheapBlock));
}

void kfreeInt(void *ptr){
    kheapBlock *block = (kheapBlock*)((uint64_t)ptr-sizeof(kheapBlock));
    checkMagic(block);
    checkRedzone(block);
    ((kheapBlock*)block->prev)->next = block->next;
    ((kheapBlock*)block->prev)->free += block->free+block->allocated+sizeof(kheapBlock)+sizeof(uint64_t);
    ((kheapBlock*)block->next)->prev = block->prev;
    block->magic = 0x0;
    if(block->chunkBorder == true){shrinkHeap(block);}
    return;
}

void *kmalloc(size_t sz){
    GET_LOCK(kmallocLock);
    void *ptr = kmallocInt(sz,false);
    FREE_LOCK(kmallocLock);
    return ptr;
}

void *kmallocAligned(size_t sz){
    GET_LOCK(kmallocLock);
    void *ptr = kmallocInt(sz,true);
    FREE_LOCK(kmallocLock);
    return ptr;
}

void kfree(void *ptr){
    GET_LOCK(kmallocLock);
    kfreeInt(ptr);
    FREE_LOCK(kmallocLock);
    return;
}

void *krealloc(void *ptr, size_t sz){
    GET_LOCK(kmallocLock);
    //Check if range can be extended
    kheapBlock *block = (kheapBlock*)((uint64_t)ptr-sizeof(kheapBlock));
    checkMagic(block);
    checkRedzone(block);
    if(block->allocated+block->free >= sz){
        block->free -= sz-block->allocated;
        block->allocated = sz;
        return ptr;
    }
    //Try allocating new memory
    void *nptr = kmallocInt(sz,false); //NOTE: Aligned allocations will break here
    if(nptr == NULL){return NULL;} //Allocation failed. The heap is likely full
    memcpy(nptr,ptr,block->allocated);
    kfreeInt(ptr);
    FREE_LOCK(kmallocLock);
    return nptr;
}