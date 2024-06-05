#pragma once
#include <stdint.h>
#include <pmm.h>

#define KHEAP_MAGIC 0xFEEF
#define KHEAP_REDZONE_MAGIC 0xABCDEFFEDCABCAFE
#define KHEAP_CALC_OFFSET(base,chunks) (base+(chunks*ORDER_MIN_BLOCK_SIZE))
#define KHEAP_REDZONE_OFFSET(block) ((uint64_t)block+sizeof(kheapBlock)+block->allocated)

typedef struct{
    uint16_t magic;
    uint32_t allocated;
    uint32_t free;
    bool chunkBorder; //True if this is the first block in a freeable chunk
    void *prev;
    void *next;
}kheapBlock;

#define KHEAP_NEWBLK_LOC(blk) ((uint64_t)blk+blk->allocated+sizeof(uint64_t)+sizeof(kheapBlock))

extern void *kmalloc(size_t sz);
extern void *kmallocAligned(size_t sz);
extern void kfree(void *ptr);
extern void *krealloc(void *ptr, size_t sz);