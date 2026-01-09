#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <pmm.h>

#define KHEAP_MAGIC 0xFEEF
#define KHEAP_REDZONE_MAGIC 0xABCDEFFEDCABCAFE
#define KHEAP_CALC_OFFSET(base,chunks) (base+(chunks*ORDER_MIN_BLOCK_SIZE))

//#define KMALLOC_DEBUG 1 //Enables debug prints for kmalloc

typedef struct{
    uint16_t magic;
    uint32_t allocated;
    uint32_t free;
    bool chunkBorder; //True if this is the first block in a freeable chunk
    void *prev;
    void *next;
}kheapBlock;

typedef struct{
    uint16_t magic;
    uint16_t line;
    char *name;
}kDebugBlock;

extern void dumpKmallocChain(uint64_t faultAddr);

extern void kfree(void *ptr);
extern void *krealloc(void *ptr, size_t sz);

#ifdef KMALLOC_DEBUG

extern void *kmallocWrapper(char *fnc, uint16_t ln, size_t sz, bool align);
#define kmalloc(sz) kmallocWrapper(__FUNCTION__,__LINE__,sz,false)
#define kmallocAligned(sz) kmallocWrapper(__FUNCTION__,__LINE__,sz,true)
#define KHEAP_BLOCK_DATA_SZ (sizeof(kheapBlock)+sizeof(kDebugBlock))

#else

#define KHEAP_BLOCK_DATA_SZ sizeof(kheapBlock)
extern void *kmallocWrapper(size_t sz, bool align);
#define kmalloc(sz) kmallocWrapper(sz,false)
#define kmallocAligned(sz) kmallocWrapper(sz,true)

#endif


//Useful macros
#define KHEAP_REDZONE_OFFSET(block) ((uint64_t)block+(KHEAP_BLOCK_DATA_SZ+block->allocated))
#define KHEAP_MAX_ALLOC_PTR(block) ((uint64_t)block+block->allocated+KHEAP_BLOCK_DATA_SZ) //Excludes redzone
#define KHEAP_MAX_TOTAL_PTR(block) ((uint64_t)block+block->free+sizeof(uint64_t)+block->allocated+KHEAP_BLOCK_DATA_SZ) //Includes redzone
#define KHEAP_ALIGNMENT_OFFSET(block) (0x1000-((KHEAP_MAX_ALLOC_PTR(block)+sizeof(uint64_t))&0xFFF)) //Returns 0x1000 if ptr is aligned
#define KHEAP_NEW_UNALIGNED_BLOCK(block) ((kheapBlock*)(KHEAP_MAX_ALLOC_PTR(block)+sizeof(uint64_t))) //Returns pointer for new block
#define KHEAP_FREE_DATA(basePtr,maxPtr,sz) (((uint64_t)maxPtr-(uint64_t)basePtr)-(KHEAP_BLOCK_DATA_SZ+sizeof(uint64_t)+sz)) //Calculates free data for new block
#define KHEAP_DEBUG_BLOCK(block) ((kDebugBlock*)((uint64_t)block+sizeof(kheapBlock))) //Returns ptr to debug block
#define KHEAP_BLOCK_FROM_PTR(ptr) ((kheapBlock*)((uint64_t)ptr-KHEAP_BLOCK_DATA_SZ))
#define KHEAP_BLOCK_DATA_PTR(block) ((void*)((uint64_t)block+KHEAP_BLOCK_DATA_SZ))