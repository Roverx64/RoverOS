#include <stdint.h>
#include <debug.h>
#include <mutex.h>
#include <mmap.h>
#include <pmm.h>
#include <bootinfo.h>
#include <string.h>
#include <kernel.h>
#include <paging.h>

uint64_t regionCount = 0;
struct memRegion *region = NULL;
uint64_t bump = 0;
mutex_t buddyLock = 0;
extern pageSpace kspace;

void *pmmBumpAlloc(struct bootInfo *kinf, size_t sz){
    if(sz+bump > kinf->mem.chunkSize){return (void*)0x0;}
    uint64_t ptr = kinf->mem.chunkPtr+bump;
    bump += sz;
    return (void*)ptr;
}

void initPMM(struct bootInfo *kinf){
    //parse mmap to get required details
    kdebug(DINFO,"Parsing mmap\n");
    size_t freeRam = 0; //Bytes of free ram we have
    size_t skippedMem = 0; //Bytes of memory wasted
    mmapEntry *entries = (mmapEntry*)kinf->mem.ptr;
    for(uint64_t i = 0; i < kinf->mem.entries; ++i){
        if(entries[i].type != MMAP_TYPE_FREE){continue;} //We ignore anything not free for use
        //If we have a block count of 0, skip
        if((entries[i].bytes/ORDER_MIN_BLOCK_SIZE) == 0){skippedMem += entries[i].bytes; continue;}
        ++regionCount;
        freeRam += entries[i].bytes;
        /*kdebug(DNONE,"Region 0x%lx with 0x%lx bytes free (0x%lx blocks) and a pbase of 0x%lx ",i,entries[i].bytes,entries[i].bytes/ORDER_MIN_BLOCK_SIZE,entries[i].phys);
        if(entries[i].phys < 0x100000){kdebug(DNONE,"(1MB)\n"); continue;}
        if(entries[i].phys < 0x100000000){kdebug(DNONE,"(LOW)\n"); continue;}
        kdebug(DNONE,"(HGH)\n");*/
    }
    kdebug(DINFO,"Skipped 0x%lx bytes (0x%lx blocks) of fragmented ram\n",skippedMem,skippedMem/ORDER_MIN_BLOCK_SIZE);
    kdebug(DINFO,"Found 0x%lx usable bytes (0x%lx blocks) of ram\n",freeRam,freeRam/ORDER_MIN_BLOCK_SIZE);
    region = (struct memRegion*)pmmBumpAlloc(kinf,sizeof(struct memRegion)*(regionCount+1));
    memset(region,0x0,sizeof(struct memRegion)*regionCount);
    //Initilize each region
    uint64_t b = 0;
    for(uint64_t i = 0; i < regionCount; ++i){
        while((entries[b].type != MMAP_TYPE_FREE) || ((entries[b].bytes/ORDER_MIN_BLOCK_SIZE) == 0)){++b;}
        region[i].base = entries[b].phys;
        region[i].limit = region[i].base+entries[b].bytes;
        region[i].blockCount = entries[b].bytes/ORDER_MIN_BLOCK_SIZE;
        //TODO: Determine best ISA and DMA spots
        if(region[i].base < 0x100000){region[i].type = MEMORY_TYPE_1MB; goto nxt;}
        if(region[i].base < 0x100000000){region[i].type = MEMORY_TYPE_LOW; goto nxt;}
        region[i].type = MEMORY_TYPE_HGH;
        nxt:
        uint64_t am = region[i].blockCount;
        if((am%2) != 0){am-=1; skippedMem += ORDER_MIN_BLOCK_SIZE; region[i].blockCount -= 1;} //Remove odd block
        //Allocate orders
        for(int o = 0; o < MAXORDER; ++o){
            uint64_t alloc = am/64;
            if(alloc == 0){alloc = 1;}
            alloc = alloc*sizeof(uint64_t);
            region[i].order[o].entryn = am;
            region[i].order[o].entry = (uint64_t*)pmmBumpAlloc(kinf,alloc);
            memset(region[i].order[o].entry,0x0,alloc);
            region[i].order[o].used = 0;
            am = am/2;
            if(am <= 0){break;}
        }
        ++b;
    }
    kdebug(DINFO,"Total wasted ram: 0x%lx\n",skippedMem);
    //Setup temporary heap for kernel paging
    //The chunk pointer will be reserved for the page heap only now
    uint64_t ptr = KALIGN((kinf->mem.chunkPtr+bump))+PAGE_SZ;
    kspace.pageHeap = ptr;
    kspace.ptr = 0;
}

//Sets an order's bit and marks the parent as allocated too
void setBit(struct memOrder *morder, uint16_t order, uint64_t idx, uint8_t bit){
    morder[order].entry[idx] |= (0x1<<bit);
    uint8_t lbit = ORDER_IS_RIGHT_BLOCK(bit) ? bit-1 : bit;
    while(order+1 < MAXORDER){
        uint8_t parentIdx = ORDER_GET_PARENT_INDX_LBIT(lbit,idx);
        uint8_t parentBit = ORDER_GET_PARENT_BIT_LBIT(lbit,idx);
        ++order;
        if(morder[order].entry == NULL){return;}
        morder[order].entry[parentIdx] |= (0x1<<(parentBit-1));
        idx = parentIdx;
        lbit = ORDER_IS_RIGHT_BLOCK(parentBit) ? parentBit-1 : parentBit;
    }
}

void freeBit(struct memOrder *morder, uint16_t order, uint64_t idx, uint8_t bit){
    uint8_t lbit = ORDER_IS_RIGHT_BLOCK(bit) ? bit : bit-1;
    while(order <= MAXORDER){
        uint8_t parentIdx = ORDER_GET_PARENT_INDX_LBIT(lbit,idx);
        uint8_t parentBit = ORDER_GET_PARENT_BIT_LBIT(lbit,idx);
        if(morder->entry[idx]&(1<<bit)){morder->entry[idx] ^= (1<<bit);}
        if(!ORDER_CAN_COALESCE(morder->entry[order],lbit)){return;}
        //Coalesce block
        lbit = parentBit;
        idx = parentIdx;
        order += 1;
    }
}

//Attempts to allocate requested memory in specified region
uint64_t allocRegion(struct memRegion *r, uint16_t order){
    //Find best order to use
    struct memOrder *morder = NULL;
    uint32_t idx = 0;
    int bit = -1;
    for(int i = order; i <= MAXORDER; ++i){
        if(r->order[i].used == r->order[i].entryn){continue;}
        morder = &r->order[i];
        break;
    }
    if(morder == NULL){return MEMORY_ALLOCATION_FAILED;}
    //Find free block in order
    for(uint32_t i = 0; i < morder->entryn; i+=64){
        idx = i/64;
        if(morder->entry[idx] == 0xFFFFFFFFFFFFFFFF){continue;}
        //Get bit
        bit = 0;
        for(;(i < morder->entryn) && (bit < 64); ++bit){
            if((morder->entry[idx]&(1<<bit)) == 0x0){break;}
            ++i;
        }
        break;
    }
    if(bit == -1){return MEMORY_ALLOCATION_FAILED;}
    setBit(morder,order,idx,bit);
    r->order[order].used += 1;
    return (uint64_t)ORDER_CALCULATE_ADDRESS(idx,bit,r->base);
}

uint64_t allocPhys(size_t bytes, uint8_t type, uint16_t flags){
    GET_LOCK(buddyLock);
    //Calculate order needed
    uint64_t sz = ORDER_MIN_BLOCK_SIZE;
    uint32_t order = 0;
    for(;order < MAXORDER; ++order){
        if(sz > bytes){break;}
        sz = sz<<order;
    }
    if(order > MAXORDER){FREE_LOCK(buddyLock); return 0x0;}
    if(type == MEMORY_TYPE_1MB){flags |= ORDER_FLAG_STRICT_MATCH;}
    //Find suitable region
    struct memRegion *bestRegion = NULL;
    for(uint64_t i = 0; i < regionCount; ++i){
        if((type != MEMORY_TYPE_1MB) && (region[i].type == MEMORY_TYPE_1MB)){continue;} //1MB memory must be asked for
        if(region[i].type == type){bestRegion = &region[i]; continue;}
        if(flags&ORDER_FLAG_STRICT_MATCH){continue;}
        if(region[i].order[order].entry == NULL){continue;} //This region cannot meet this demand
        if(bestRegion == NULL){bestRegion = &region[i]; continue;} //Take anything
        //Check if type is between ISA and 1MB
        if((region[i].type > MEMORY_TYPE_1MB) && (region[i].type < MEMORY_TYPE_ISA)){
            bestRegion = &region[i];
            continue;
        }
        //TODO: Check regions sizes and best fit region
        //NOTE: This may fail even if memory is available
    }
    if(bestRegion == NULL){kpanic("No memory available",0);}
    //Allocate block
    uint64_t ptr = allocRegion(bestRegion,order);
    FREE_LOCK(buddyLock);
    return ptr;
}

void freePhys(uint64_t ptr){
    GET_LOCK(buddyLock);
    //Find region
    struct memRegion *reg = NULL;
    for(uint64_t i = 0; i < regionCount; ++i){
        if((region[i].base > ptr) && (ptr < region[i].limit)){
            reg = &region[i];
            break;
        }
    }
    uint16_t order = 0;
    uint8_t bit = ORDER_ADDRESS_TO_BIT(ptr,reg->base);
    uint64_t idx = ORDER_GET_IDX_FROM_BIT(bit);
    freeBit(&region->order,order,idx,bit);
    FREE_LOCK(buddyLock);
    return;
}