#include <kmalloc.h>
#include <mutex.h>
#include <stdint.h>
#include <stdbool.h>
#include <kernel.h>
#include <string.h>
#include <task.h>
#include "kprint.h"
#include "paging.h"
#include "pmm.h"
#include "vmm.h"

extern pageSpace kspace;
extern uint64_t linkerHeapRegion;

kheapBlock *startBlock = NULL;
kDebugBlock *dbgBlock = NULL;
size_t bytesFree = 0; //Auto-extend heap when low enough
mutex_t kmallocLock = 0;

/*
    !D Initilizes the kernel's heap allocator
    !I heapStart: Virtual address of heap's start
    !I size: Initial heap size of 'heapStart'
    !R None
    !C NOTE: This function must be called before any heap allocation is made.
*/
void initKmalloc(uint64_t heapStart, uint64_t size){
    kinfo("Kernel heap at 0x%lx\n",heapStart);
    startBlock = (kheapBlock*)heapStart;
    startBlock->magic = KHEAP_MAGIC;
    startBlock->allocated = 0x0;
    startBlock->free = KHEAP_FREE_DATA(heapStart,heapStart+size,0);
    startBlock->chunkBorder = false;
    startBlock->prev = NULL;
    startBlock->next = NULL;
    #ifdef KMALLOC_DEBUG
    dbgBlock = KHEAP_DEBUG_BLOCK(startBlock);
    dbgBlock->magic = KHEAP_MAGIC;
    dbgBlock->line = __LINE__;
    dbgBlock->name = "kmalloc";
    #endif
    uint64_t *rdz = (uint64_t*)KHEAP_REDZONE_OFFSET(startBlock);
    *rdz = KHEAP_REDZONE_MAGIC;
}

#ifdef KMALLOC_DEBUG

/*!
    !D Adds debug information to kernel heap blocks
    !I block: Heap block to add debug info to
    !I fnc: Function name to write
    !I line: Function line to write
    !R None
    !C NOTE: This function is only availible when 'KMALLOC_DEBUG' is defined
*/
void addDebugBlock(kheapBlock *block, char *fnc, uint16_t line){
    kDebugBlock *dbg = KHEAP_DEBUG_BLOCK(block);
    dbg->magic = KHEAP_MAGIC;
    dbg->name = fnc;
    dbg->line = line;
}

#endif

/*!
    !D Extends the kernel heap by 'min' bytes
    !I min: Minimum bytes (Rounds up to the ORDER block size that can contain 'min' bytes)
    !R kheapBlock at new memory on success, NULL on failure
*/
static kheapBlock *extendHeap(uint64_t min){
    //ALIGN and add a page if using min
    uint32_t pages = (min/PAGE_SZ);
    min = (pages+5)*PAGE_SZ;
    kheapBlock *nblock = (kheapBlock*)vmalloc(min,VTYPE_HEAP,VFLAG_WRITE|VFLAG_KERNEL|VFLAG_MAKE);
    KASSERT((nblock != NULL),"Failed to extend kernel heap");
    kinfo("Extended kheap with new mem at 0x%lx with 0x%lx bytes (max 0x%lx)\n",(uint64_t)nblock,min,(uint64_t)nblock+min);
    nblock->allocated = 0;
    nblock->free = min-(KHEAP_BLOCK_DATA_SZ+sizeof(uint64_t));
    nblock->chunkBorder = true;
    nblock->prev = NULL;
    nblock->next = NULL;
    return nblock;
}

/*!
    !D Shrinks the kernel heap
    !I block: Block at the start of memory to free
    !R None
    !C TODO: use freeNPhys() over freePhys()
*/
static void shrinkHeap(kheapBlock *block){
    if(block->chunkBorder != true){return;}
    vmFree(block);
    kinfo("Shrank heap at 0x%lx\n",(uint64_t)block);
}

/*!
    !D checks a kheapBlock's redzone
    !I block: kheapBlock to check
    !R None
    !C NOTE: On failure, this function kernel panics
*/
static inline void checkRedzone(kheapBlock *block){
    uint64_t *rdz = (uint64_t*)KHEAP_REDZONE_OFFSET(block);
    #ifdef KMALLOC_DEBUG
    if(*rdz != KHEAP_REDZONE_MAGIC){
        kinfo("Block at 0x%lx failed redzone check! (A:0x%lx|F:0x%x|R:0x%x)\n",(uint64_t)block,block->allocated,block->free,(uint64_t)*rdz);
    }
    #endif
    if((*rdz != KHEAP_REDZONE_MAGIC)){dumpKmallocChain(~0x0);}
    KASSERT((*rdz == KHEAP_REDZONE_MAGIC),"Corrupted redzone");
}

/*!
    !D checks a kheapBlock's magic
    !I block: kheapBlock to check
    !R None
    !C NOTE: On failure, this function kernel panics
*/
static inline void checkMagic(kheapBlock *block){
    #ifdef KMALLOC_DEBUG
    if(block->magic != KHEAP_MAGIC){
        kinfo("Block at 0x%lx failed magic check! (A:0x%lx|F:0x%x|M:0x%x)\n",(uint64_t)block,block->allocated,block->free,(uint32_t)block->magic);
    }
    #endif
    if((block->magic != KHEAP_MAGIC)){dumpKmallocChain(~0x0);}
    KASSERT((block->magic == KHEAP_MAGIC),"Invalid kblock magic");
}

extern void dumpKmallocBlock(kheapBlock *block, bool rdz);

/*!
    !D Internal kmalloc function
    !I sz: Size to allocate
    !I align: Align block to page boundary
    !R Returns pointer to block of data on success, or NULL on failure
    !C NOTE: This function should never be called directly
    !C LOCK: This function does not get the kheap LOCK and assumes caller owns the lock
*/
void *kmallocInt(size_t sz, bool align){
    //Align allocates on a 4KB boundary
    kheapBlock *block = startBlock;
    kheapBlock *nblock = NULL;
    bool split = false;
    size_t csz = sz+sizeof(uint64_t)+KHEAP_BLOCK_DATA_SZ;
    if(sz == 0){return NULL;} 
    #ifdef KMALLOC_DEBUG
    //kinfo("Starting allocation for 0x%lx bytes; True size 0x%lx; Align(0x%x)\n",sz,csz,(uint32_t)align);
    #endif
    for(;;){
        checkMagic(block);
        if(align){
            //Check if block can be split and contain aligned address
            uint64_t off = KHEAP_ALIGNMENT_OFFSET(block);
            //Check if we can fit block data in offset
            if(off < (KHEAP_BLOCK_DATA_SZ)){continue;}
            //Check if block has space for data and alignment
            if(block->free >= (csz+off)){split = true; break;}
            //No space :(
        }
        if((block->free >= csz) && (!align)){split = true; break;}
        if(block->next == NULL){break;}
        block = (kheapBlock*)block->next;
    }
    checkRedzone(block);
    if(align && split){
        //dumpKmallocBlock(block,true);
        uint64_t basePtr = KHEAP_MAX_ALLOC_PTR(block)+sizeof(uint64_t);
        uint64_t offset = KHEAP_ALIGNMENT_OFFSET(block);
        uint64_t alignedPtr = basePtr+offset;
        uint64_t maxPtr = KHEAP_MAX_TOTAL_PTR(block);
        uint64_t nblockPtr = alignedPtr-KHEAP_BLOCK_DATA_SZ;
        nblock = (kheapBlock*)nblockPtr;
        nblock->free = KHEAP_FREE_DATA(nblock,maxPtr,sz);
        nblock->allocated = sz;
        nblock->prev = (void*)block;
        nblock->next = block->next;
        if(block->next){((kheapBlock*)nblock->next)->prev = (void*)nblock;}
        block->free = KHEAP_FREE_DATA(block,nblock,block->allocated);
        uint64_t alignedDataPtr = (uint64_t)KHEAP_BLOCK_DATA_PTR(nblock);
        goto end;
    }
    if(split){
        nblock = (kheapBlock*)KHEAP_NEW_UNALIGNED_BLOCK(block);
        nblock->allocated = sz;
        nblock->free = KHEAP_FREE_DATA((uint64_t)nblock,KHEAP_MAX_TOTAL_PTR(block),sz);
        nblock->chunkBorder = false;
        nblock->prev = (void*)block;
        nblock->next = block->next;
        block->next = (void*)nblock;
        block->free = 0;
        if(nblock->next != NULL){((kheapBlock*)nblock->next)->prev = (void*)nblock;}
        goto end;
    }
    //Heap needs more space
    nblock = extendHeap(csz);
    KASSERT((nblock->free >= sz),"extendHeap() did not allocate enough memory");
    nblock->allocated = sz;
    nblock->free -= sz;
    block->next = (void*)nblock;
    nblock->prev = (void*)block;
    nblock->chunkBorder = true;
    uint64_t maxPtr = KHEAP_MAX_TOTAL_PTR(block);
    end:
    uint64_t *rdz = (uint64_t*)KHEAP_REDZONE_OFFSET(nblock);
    uint64_t mcalc = (uint64_t)nblock+KHEAP_BLOCK_DATA_SZ+nblock->allocated;
    *rdz = KHEAP_REDZONE_MAGIC;
    nblock->magic = KHEAP_MAGIC;
    #ifdef KMALLOC_DEBUG
    //kinfo("Allocated new block (0x%lx)(0x%lx){M:0x%x|A:0x%x|F:0x%x}\n",(uint64_t)nblock,(uint64_t)KHEAP_DEBUG_BLOCK(nblock),(uint64_t)nblock->magic,nblock->allocated,nblock->free);
    #endif
    return (void*)KHEAP_BLOCK_DATA_PTR(nblock);
}

/*!
    !D Internal kfree function
    !I ptr: pointer to free
    !R None
    !C NOTE: This function should never be called directly
    !C LOCK: This function does not get the kheap LOCK and assumes caller owns the lock
*/
void kfreeInt(void *ptr){
    kheapBlock *block = KHEAP_BLOCK_FROM_PTR(ptr);
    checkMagic(block);
    checkRedzone(block);
    #ifdef KMALLOC_DEBUG
    kinfo("Freeing block at (0x%lx) with (0x%x) bytes alloc'd\n",(uint64_t)block,block->allocated);
    #endif
    kheapBlock *pblock = (kheapBlock*)block->prev;
    if(!block->chunkBorder){
        //Add memory back to previous block
        pblock->free += sizeof(kheapBlock)+block->allocated+block->free+sizeof(uint64_t);
    }
    pblock->next = block->next;
    if(block->next){
        ((kheapBlock*)block->next)->prev = block->prev;
    }
    block->magic = 0x0;
    if(block->chunkBorder == true){shrinkHeap(block);}
    return;
}

/*!
    !D Frees a block of memory from the kernel heap
    !I ptr: pointer to free
    !R None
    !C LOCK: This function aquires the kheap lock
*/
void kfree(void *ptr){
    GET_LOCK(kmallocLock);
    kfreeInt(ptr);
    FREE_LOCK(kmallocLock);
    return;
}

/*!
    !D Reallocates a block of memory to new size
    !I ptr: Pointer to allocated block of data
    !I sz: New size to allocate
    !R Pointer to new block of data on success, or NULL on failure
    !C LOCK: This function aquires the heap lock
*/
void *krealloc(void *ptr, size_t sz){
    GET_LOCK(kmallocLock);
    //Check if range can be extended
    kheapBlock *block = KHEAP_BLOCK_FROM_PTR(ptr);
    checkMagic(block);
    checkRedzone(block);
    if(block->allocated == sz){return ptr;}
    #ifdef KMALLOC_DEBUG
    kDebugBlock *dblk = (kDebugBlock*)KHEAP_DEBUG_BLOCK(block);
    sz += sizeof(kDebugBlock);
    #endif
    if(block->allocated+block->free >= sz){
        uint64_t diff = (sz > block->allocated) ? sz-block->allocated : block->allocated-sz;
        if(sz > block->allocated){
            //Expand upwards
            block->free -= diff;
            block->allocated += diff;
        }
        else{
            //Expand downwards
            block->free += diff;
            block->allocated -= diff;
        }
        //Move redzone
        *((uint64_t*)KHEAP_REDZONE_OFFSET(block)) = KHEAP_REDZONE_MAGIC;
        FREE_LOCK(kmallocLock);
        return ptr;
    }
    //Try allocating new memory
    void *nptr = kmallocInt(sz,false); //NOTE: Aligned allocations will break here
    if(nptr == NULL){FREE_LOCK(kmallocLock); return NULL;} //Allocation failed. The heap is likely full
    #ifdef KMALLOC_DEBUG
    void *db1 = (void*)((uint64_t)block+sizeof(kheapBlock));
    memcpy(nptr,db1,sizeof(kDebugBlock));
    memset(db1,0x0,sizeof(kDebugBlock));
    nptr = (void*)((uint64_t)nptr+sizeof(kDebugBlock));
    #endif
    memcpy(nptr,ptr,block->allocated);
    kfreeInt(ptr);
    FREE_LOCK(kmallocLock);
    return nptr;
}

/*!
    !D Dumps information about a kheap block
    !I block: kheapBlock to dump
    !I rdz: Prints redzone data when true
    !R None
*/
void dumpKmallocBlock(kheapBlock *block, bool rdz){
    uint64_t *red = (uint64_t*)KHEAP_REDZONE_OFFSET(block);
    knone("+========(0x%lx)========+\n",(uint64_t)block);
    if(block->magic != KHEAP_MAGIC){knone("(CORRUPTED MAGIC)\n");}
    if(rdz){if(*red != KHEAP_REDZONE_MAGIC){knone("(CORRUPTED REDZONE)\n");}}
    #ifdef KMALLOC_DEBUG
    kDebugBlock *dblk = KHEAP_DEBUG_BLOCK(block);
    if(dblk->magic != KHEAP_MAGIC){knone("Corrupted debug magic\n");}
    else{knone("Created by: %s on line: 0x%lx\n",dblk->name,(uint64_t)dblk->line);}
    #endif
    knone("Magic: 0x%x\n",(uint32_t)block->magic);
    knone("Allocated: 0x%x\n",block->allocated);
    knone("Free: 0x%x\n",block->free);
    knone("Address: 0x%lx\n",(uint64_t)block+KHEAP_BLOCK_DATA_SZ);
    knone("Border: ");
    if(block->chunkBorder){knone("True\n");}else{knone("False\n");}
    knone("Prev: 0x%lx\n",(uint64_t)block->prev);
    knone("Next: 0x%lx\n",(uint64_t)block->next);
    knone("CRDZA: 0x%lx\n",(uint64_t)KHEAP_REDZONE_OFFSET(block));
    if(rdz){
        knone("Redzone: 0x%lx\n",(uint64_t)(*red));
    }
    if((uint64_t)red < ((uint64_t)block+KHEAP_BLOCK_DATA_SZ+block->allocated)){
        knone("Redzone is inside of kheap block data\n");
    }
}

/*!
    !D Dumps information about all allocations on fault
    !I faultAddr: Faulting address
    !R None
*/
void dumpKmallocChain(uint64_t faultAddr){
    kheapBlock *block = startBlock;
    bool ignAddr = false;
    if(faultAddr < (uint64_t)startBlock){ignAddr = true;}
    while(block != NULL){
        //Check if redzone will cause a fault
        bool doRdz = true;
        if((KHEAP_REDZONE_OFFSET(block) >= faultAddr) && (!ignAddr)){doRdz = false;}
        dumpKmallocBlock(block,doRdz);
        block = (kheapBlock*)block->next;
        //Checks to avoid a page fault
        if(((uint64_t)block >= faultAddr) && (!ignAddr)){return;}
        if((((uint64_t)block+sizeof(kheapBlock)) >= faultAddr) && (!ignAddr)){return;}
    }
}

#ifdef KMALLOC_DEBUG

void *kmallocWrapper(char *fnc, uint16_t ln, size_t sz, bool align){
    GET_LOCK(kmallocLock);
    void *ptr = NULL;
    ptr = kmallocInt(sz,align);
    if(!ptr){FREE_LOCK(kmallocLock); return NULL;}
    kheapBlock *blk = KHEAP_BLOCK_FROM_PTR(ptr);
    addDebugBlock(blk,fnc,ln);
    FREE_LOCK(kmallocLock);
    return ptr;
}

#else

/*!
    !D Kmalloc wrapper
    !I sz: size to allocate
    !I align: Align block to page boundary
    !R Pointer to allocated memory on success, or NULL on failure
    !C NOTE: This function should be called via provided macros
    !C LOCK: This function aquires the kheap lock
*/
void *kmallocWrapper(size_t sz, bool align){
    GET_LOCK(kmallocLock);
    void *ptr = kmallocInt(sz,align);
    if(!ptr){FREE_LOCK(kmallocLock); return NULL;}
    FREE_LOCK(kmallocLock);
    return (void*)ptr;
}

#endif