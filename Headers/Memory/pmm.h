#pragma once
#include <stdint.h>
#include "bootinfo.h"

#define MAXORDER 10
#define ORDER_MIN_BLOCK_SIZE 0x4000 //16KB chunks
#define ORDER_BLOCK_SIZE(o) ((0x1<<o)*ORDER_MIN_BLOCK_SIZE)
#define PTR_IS_NOT_MIN_BLOCK_ALIGNED(ptr) ((uint64_t)ptr&0x3FFF)

#define MEMORY_TYPE_ANY 0x0 //Use any memory available
#define MEMORY_TYPE_1MB 0x1 //Use memory below 1MB
#define MEMORY_TYPE_LOW 0x2 //Use memory below 4GB
#define MEMORY_TYPE_HGH 0x3 //Use memory above 4GB
#define MEMORY_TYPE_ISA 0x4 //Use memory suitable for ISA
#define MEMORY_TYPE_DMA 0x5 //Use memory suitable for DMA

#define MEMORY_ALLOCATION_FAILED 0xFFF

//Tree management&utils
#define ORDER_GET_LCHILD_BIT(blk) (blk<<1)
#define ORDER_GET_RCHILD_BIT(blk) ((blk<<1)+1)
#define ORDER_GET_PARENT_LBIT(blk) (blk>>1)
#define ORDER_GET_PARENT_RBIT(blk) ((blk>>1)-1)
#define ORDER_CAN_COALESCE(bmp,lblk) ((bmp&lblk)|((lblk>>1)&bmp))
#define ORDER_IS_RIGHT_BLOCK(blk) (((blk+2)%0x2)&0x1)
#define ORDER_GET_PARENT_INDX_LBIT(lblk,idx) ((((idx*64)+lblk)/2)/64)
#define ORDER_GET_PARENT_BIT_LBIT(lblk,idx) (((idx*64)+lbit)/2)
#define ORDER_ADDRESS_TO_BIT(addr,base) ((addr-base)/ORDER_MIN_BLOCK_SIZE)
#define ORDER_CALCULATE_ADDRESS(idx,bit,base) (base+(((idx*64)+bit)*ORDER_MIN_BLOCK_SIZE))
#define ORDER_GET_IDX_FROM_BIT(bit) (bit/64)

//(((I*64)+L)/2)

#define ORDER_FLAG_STRICT_MATCH 1<<0 //Memory type must match requested type

struct memOrder{
    uint32_t entryn; //Total number of blocks
    uint32_t used; //Used number of blocks
    uint64_t *entry; //Bitmap array
};

struct memRegion{
    uint8_t type; //Memory type
    uint64_t base; //Base phys addr of region
    uint64_t limit; //Highest address in region
    uint64_t blockCount; //Number of free MIN_BLOCK_SIZE blocks
    struct memOrder order[MAXORDER];
};

extern void initPMM(struct bootInfo *kinf);
extern uint64_t allocPhys(size_t bytes, uint8_t type, uint16_t flags);
extern void freePhys(uint64_t ptr);