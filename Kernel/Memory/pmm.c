#include <stdint.h>
#include <kprint.h>
#include <mutex.h>
#include <mmap.h>
#include <pmm.h>
#include <bootinfo.h>
#include <string.h>
#include <kernel.h>
#include <paging.h>
#include <kmalloc.h>

uint64_t regionCount = 0;
struct memRegion *region = NULL;
mutex_t buddyLock = 0;
extern pageSpace kspace;

extern uint64_t __heapStart;
extern uint64_t __heapEnd;
extern void initKmalloc(uint64_t heapStart,uint64_t heapSize);
extern void initSLAB(struct bootInfo *kinf);
extern void createFakeTask();
extern void initPaging();

void dumpRegions(){
    for(int i = 0; i < regionCount; ++i){
        kinfo("Region[0x%x]->order[0x%x]->entry(0x%lx): (0x%lx)\n",(uint32_t)i,(uint32_t)0,(uint64_t)region[i].order[0].entry,(uint64_t)region[i].order[0].entryn);
    }
}

uint8_t setRegionType(mmapEntry *ent){
    if(ent->phys < 0x100000){return MEMORY_TYPE_1MB;}
    if(ent->phys < 0x100000000){return MEMORY_TYPE_LOW;}
    return MEMORY_TYPE_HGH;
}

char *rtypes[6] = {
    "ANY", "1MB", "LOW",
    "HGH", "ISA", "DMA"
};

bool shouldInit = true;

/*!
    !D Creates a region for an mmap entry and sets bits if an allocation is in the region
    !I rn: region number to use \
    entry: mmap entry to parse \
    kinf: kernel information structure
    !R None
    !C LOCK: No mutex is used for this function
*/
void createRegion(uint32_t rn, mmapEntry *entry, struct bootInfo *kinf){
    mmapEntry *amap = (mmapEntry*)kinf->mem.aptr;
    region[rn].type = setRegionType(entry);
    region[rn].base = entry->phys;
    region[rn].limit = entry->phys+entry->bytes;
    region[rn].blockCount = (entry->bytes)/ORDER_MIN_BLOCK_SIZE;
    if(region[rn].blockCount%2){region[rn].blockCount -= 1;} //Remove odd block
    uint64_t blc = region[rn].blockCount;
    //Allocate orders
    uint32_t order = 0;
    while((blc > 0) && (order < MAXORDER)){
        region[rn].order[order].entryn = blc;
        region[rn].order[order].used = 0;
        uint64_t alloc = ((blc/64) == 0x0) ? sizeof(uint64_t) : (blc/64)*sizeof(uint64_t);
        region[rn].order[order].entry = (struct memOrder*)kmalloc(alloc);
        memset(region[rn].order[order].entry,0x0,alloc);
        //knone("  \u21B3Order[0x%x][0x%x blocks][0x%lx bytes]->(0x%lx)\n",order,region[rn].order[order].entryn,alloc,(uint64_t)region[rn].order[order].entry);
        blc = blc/2;
        ++order;
    }
    kinfo("Created region [%s][Phys 0x%lx][0x%lx blocks][0x%lx orders]\n",rtypes[region[rn].type],region[rn].base,region[rn].blockCount,order);
    region[rn].ordern = order;
    region[rn].flags |= REGION_FLAG_COMPLETE;
    if((region[rn].type >= MEMORY_TYPE_LOW) && (shouldInit == true)){
        shouldInit = false;
    }
    //Check for allocations in this region
    for(uint32_t z = 0; z < kinf->mem.aentries; ++z){
        if(amap[z].phys < entry->phys){continue;}
        if((amap[z].phys+amap[z].bytes) > (entry->phys+entry->bytes)){continue;}
        //kinfo("Found allocation entry 0x%x [%s] in region 0x%x\n",z,mmapTypeString[amap->type],rn);
        //Check order needed and allocate it
        allocPhysAddress(amap[z].phys,amap[z].bytes);
    }
}

extern void initKsym(void *ksym);
extern void initVmalloc();

/*!
    !D Intilizes the physical memory manager
    !I kinf: kernel information structure
    !R None
    !C LOCK: No mutex is used for this function
*/
void initPMM(struct bootInfo *kinf){
    size_t freeRam = 0; //Bytes of free ram we have
    size_t skippedMem = 0; //Bytes of memory wasted
    mmapEntry *entries = (mmapEntry*)kinf->mem.ptr;
    //Initilize kmalloc()
    initKmalloc((uint64_t)&__heapStart,((uint64_t)&__heapEnd-(uint64_t)&__heapStart));
    //Initilze temporary valloc()
    createFakeTask();
    //Setup paging
    initPaging();
    //Init ksym
    initKsym((void*)kinf->mem.symptr);
    //parse mmap to get required details
    kinfo("Parsing mmap\n");
    for(uint64_t i = 0; i < kinf->mem.entries; ++i){
        if(entries[i].type != MMAP_TYPE_FREE){continue;} //We ignore anything not free for use
        kinfo("MMAP[%s][Phys 0x%lx][Bytes 0x%lx][Max 0x%lx]\n",mmapTypeString[entries[i].type],entries[i].phys,entries[i].bytes,entries[i].phys+entries[i].bytes);
        //If we have a block count of 0, skip
        if((entries[i].bytes/ORDER_MIN_BLOCK_SIZE) == 0){skippedMem += entries[i].bytes; continue;}
        ++regionCount;
        freeRam += entries[i].bytes;
    }
    kinfo("Skipped 0x%lx bytes (0x%lx blocks) of fragmented ram\n",skippedMem,skippedMem/ORDER_MIN_BLOCK_SIZE);
    kinfo("Found 0x%lx usable bytes (0x%lx blocks) of ram (0x%lx regions)\n",freeRam,freeRam/ORDER_MIN_BLOCK_SIZE,regionCount);
    region = (struct memRegion*)kmalloc(sizeof(struct memRegion)*(regionCount));
    memset(region,0x0,sizeof(struct memRegion)*regionCount);
    //Find HIGH MEM or LOW MEM region to initilize first for kheap extensions
    uint32_t frn = 0;
    for(uint64_t i = 0; i < kinf->mem.entries; ++i){
        if(entries[i].type != MMAP_TYPE_FREE){continue;}
        ++frn;
        uint8_t rt = setRegionType(&entries[i]);
        if((rt != MEMORY_TYPE_LOW) && (rt != MEMORY_TYPE_HGH)){continue;}
        createRegion(frn,&entries[i],kinf);
        entries[i].flags |= MMAP_FLAG_PARSED;
        break;
    }
    initSLAB(kinf);
    initVmalloc();
    //Initilize each region
    uint32_t rn = 0;
    for(uint64_t i = 0; i < kinf->mem.entries; ++i){
        if(entries[i].type != MMAP_TYPE_FREE){continue;}
        if(entries[i].flags&MMAP_FLAG_PARSED){continue;}
        createRegion(rn,&entries[i],kinf);
        ++rn;
        if(region[rn].type >= MEMORY_TYPE_LOW){shouldInit = true;}
    }
    kinfo("Total wasted ram: 0x%lx\n",skippedMem);
}

/*!
    !D Sets an order's bit and marks the parents as allocated
    !I r: memory region to use
    !I order: order to set
    !I idx: index block resides in
    !I bit: bit of block being set
    !R None
    !C LOCK: Function assumes lock has been obtained before being called
*/
void setBit(struct memRegion *r, uint16_t order, uint64_t idx, uint8_t bit){
    //kinfo("Setting bit [O 0x%x][I 0x%lx][B 0x%x]->[0x%lx]\n",(uint32_t)order,idx,(uint32_t)bit,r->order[order].entry[idx]);
    uint64_t childBaseBit = ORDER_GET_CHILD_NUM(order,bit);
    childBaseBit = (ORDER_IS_RIGHT_BLOCK(childBaseBit)) ? childBaseBit-1 : childBaseBit;
    uint64_t childBaseIdx = ORDER_GET_IDX_FROM_BIT(childBaseBit);
    childBaseBit = childBaseBit/64; //Get local entry bit
    //Set child bits until we reach the parent order
    for(uint32_t i = 0; i < order; ++i){
        //kinfo("Setting C[O 0x%x][I 0x%x][B 0x%x]\n",i,childBaseIdx,childBaseBit);
        r->order[i].entry[childBaseIdx] |= (1<<childBaseIdx);
        r->order[i].used += 2;
        childBaseBit = ORDER_GET_PARENT_BIT_LBIT(childBaseBit,childBaseIdx);
        childBaseIdx = ORDER_GET_IDX_FROM_BIT(childBaseBit);
        childBaseBit = childBaseBit%64;
    }
    //Set parents above block
    r->order[order].entry[idx] |= (1<<bit);
    r->order[order].used += 1;
    bit = ORDER_IS_RIGHT_BLOCK(bit) ? bit-1 : bit;
    bit = ORDER_GET_PARENT_BIT_LBIT(bit,idx);
    idx = ORDER_GET_IDX_FROM_BIT(bit);
    bit = bit/64;
    ++order;
    while(order < r->ordern){
        if(r->order[order].entryn == 0){break;}
        r->order[order].entry[idx] |= (1<<bit);
        r->order[order].used += 1;
        //kinfo("Setting [O 0x%x][I 0x%x][B 0x%x]->[0x%lx]\n",order,idx,bit,r->order[order].entry[idx]);
        ++order;
        bit = ORDER_GET_PARENT_BIT_LBIT(bit,idx);
        idx = ORDER_GET_IDX_FROM_BIT(bit);
        bit = bit%64;
    }
}

/*!
    !D Free's a order and it's children/parents
    !I r: memory region to use
    !I order: order to set
    !I idx: index block resides in
    !I bit: bit of block being set
    !R None
    !C LOCK: Function assumes lock has been obtained before being called
*/
void freeBit(struct memRegion *r, uint16_t order, uint64_t idx, uint8_t bit){
    uint8_t lbit = ORDER_IS_RIGHT_BLOCK(bit) ? bit : bit-1;
    uint8_t test = ORDER_IS_RIGHT_BLOCK(bit) ? lbit : bit+1;
    r->order[order].entry[idx] &= ~(1<<bit);
    if((r->order[order].entry[idx]&(1<<test)) != 0x0){return;} //Parents must stay allocated
    ++order;
    uint8_t parentIdx = idx;
    uint8_t parentBit = bit;
    //Free parent bits if other bit is not set
    while(order < MAXORDER){
        parentBit = ORDER_GET_PARENT_BIT_LBIT(parentBit,parentIdx);
        parentIdx = ORDER_GET_IDX_FROM_BIT(parentBit);
        parentBit = parentBit%64;
        r->order[order].entry[parentIdx] &= ~(0x1<<parentBit);
        if((r->order[order].entry[parentIdx]&(0x1<<(parentBit+1))) != 0x0){return;}
        //Coalese parents
        ++order;
    }
}

/*!
    !D Attempts to allocate requested memory in specified region
    !I r: memory region to use
    !I order: desired order to allocate
    !R Returns pointer to physical memory on success
    !R Returns `MEMORY_ALLOCATION_FAILED` on failure
    !C LOCK: Function assumes lock has been obtained before being called
*/
uint64_t allocRegion(struct memRegion *r, uint16_t order){
    //Find best order to use
    struct memOrder *morder = NULL;
    uint32_t idx = 0;
    int bit = -1;
    morder = &r->order[order];
    //If specified order is fully allocated, then the orders above are filled too
    if(morder->used >= morder->entryn){return MEMORY_ALLOCATION_FAILED;}
    //Find free block in order
    for(uint32_t i = 0; i < morder->entryn; i+=64){
        idx = i/64;
        //kinfo("Check [I 0x%x]->[0x%lx]\n",idx,morder->entry[idx]);
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
    //kinfo("Using region (0x%lx)[O: 0x%x][I: 0x%x][B: 0x%x]->0x%lx\n",(uint64_t)r,(uint32_t)order,idx,(uint32_t)bit,ORDER_CALCULATE_ADDRESS(idx,bit,r->base));
    uint64_t base = ORDER_GET_CHILD_NUM(order,bit);
    base *= ORDER_MIN_BLOCK_SIZE;
    base += r->base;
    uint64_t tbit = (idx*64)+bit;
    //kinfo("[Base 0x%lx][B 0x%x][I 0x%x][OCA 0x%lx]\n",r->base,bit,idx,ORDER_CALCULATE_ADDRESS(idx,bit,r->base));
    //kinfo("ABS 0x%lx [0x%lx]\n",ORDER_CALCULATE_ABS_ADDRESS(order,tbit,r->base));
    setBit(r,order,idx,bit);
    return (uint64_t)ORDER_CALCULATE_ABS_ADDRESS(order,bit,r->base);
}

/*!
    !D Allocates physical memory
    !I bytes: bytes to allocate
    !I type: type of memory to allocate
    !I flags: flags
    !I actual: Actual size allocated
    !R Returns pointer to physical address on success
    !R Returns `MEMORY_ALLOCATION_FAILED` on failure
    !C LOCK: This function obtains lock upon being called
    !C NOTE: this function may allocate more bytes than requested
*/
uint64_t allocPhys(size_t bytes, uint8_t type, uint16_t flags, size_t *actual){
    GET_LOCK(buddyLock);
    *actual = 0x0;
    //Calculate order needed
    uint64_t sz = ORDER_MIN_BLOCK_SIZE;
    uint32_t order = 0;
    if((flags&ORDER_FLAG_STRICT_MATCH) && (type == MEMORY_TYPE_ANY)){
        kwarn("Conflicting flag 'ORDER_STRICT_MATCH' with type of 'ANY'; Discarding strict flag\n");
        flags &= ~(ORDER_FLAG_STRICT_MATCH);
    }
    for(;order < MAXORDER; ++order){
        if(sz >= bytes){break;}
        sz = sz<<order;
    }
    if(order > MAXORDER){FREE_LOCK(buddyLock); return MEMORY_ALLOCATION_FAILED;}
    if(type == MEMORY_TYPE_1MB){flags |= ORDER_FLAG_STRICT_MATCH;}
    //Find suitable region
    struct memRegion *bestRegion = NULL;
    uint64_t i = 0;
    uint64_t use = 0;
    for(; i < regionCount; ++i){
        if(region[i].flags&REGION_FLAG_COMPLETE == 0x0){continue;}
        if((type != MEMORY_TYPE_1MB) && (region[i].type == MEMORY_TYPE_1MB)){continue;} //1MB memory must be asked for
        if((flags&ORDER_FLAG_STRICT_MATCH) && (region[i].type != type)){continue;}
        if(region[i].order[order].entry == NULL){/*kinfo("Cannot meet demand 0x%lx\n",i);*/ continue;} //This region cannot meet this demand
        if(region[i].type == type){bestRegion = &region[i]; use = i; continue;}
        if(bestRegion == NULL){bestRegion = &region[i]; use = i; continue;} //Take anything
        //Check if type is between ISA and 1MB
        if((region[i].type > MEMORY_TYPE_1MB) && (region[i].type < MEMORY_TYPE_ISA)){
            if(i == 0x3){break;}
            bestRegion = &region[i];
            use = i;
            continue;
        }
        //TODO: Check regions sizes and best fit region
        //NOTE: This may fail even if memory is available
    }
    if(bestRegion == NULL){kpanic("No memory available",bytes);}
    //Allocate block
    uint64_t ptr = allocRegion(bestRegion,order);
    //kinfo("Allocated 0x%lx bytes - 0x%x\n",ORDER_BLOCK_SIZE(order),order);
    *actual = ORDER_BLOCK_SIZE(order);
    //kinfo("Allocated to 0x%lx\n",ptr);
    FREE_LOCK(buddyLock);
    return ptr;
}

/*!
    !D Marks an address as allocated
    !I addr: Address to mark as allocated
    !I bytes: Bytes of memory to reserve
    !R Returns `addr` on success
    !R Returns `MEMORY_ALLOCATION_FAILED` on failure
    !C LOCK: This function obtains lock upon being called
*/
uint64_t allocPhysAddress(uint64_t addr, size_t bytes){
    GET_LOCK(buddyLock);
    uint64_t rn = 0;
    struct memRegion *r = NULL;
    while(rn < regionCount){
        if((region[rn].base < addr) && (region[rn].limit > addr)){r = &region[rn]; break;}
        ++rn;
    }
    if(!r){kwarn("Invalid phys addr 0x%lx\n",addr); FREE_LOCK(buddyLock); return MEMORY_ALLOCATION_FAILED;}
    size_t blockSz = ORDER_MIN_BLOCK_SIZE;
    uint64_t ptr = KALIGN(addr);
    //Recursively set the order if needed
    while(bytes != 0){
        uint64_t bit = ORDER_ADDRESS_TO_BIT(ptr,r->base);
        uint32_t idx = ORDER_GET_IDX_FROM_BIT(bit);
        bit = bit%64;
        setBit(r,0,idx,bit);
        //knone("ptr 0x%lx [0x%lx bytes][0x%lx rel]\n",ptr,bytes,ptr-r->base);
        bytes = (bytes > blockSz) ? (bytes-blockSz) : 0;
        ptr += blockSz;
    }
    FREE_LOCK(buddyLock);
    return addr;
}

/*!
    !D Frees a physical address of `ORDER_MIN_BLOCK_SIZE` size
    !I ptr: Physical address to free
    !R none
    !C LOCK: This function obtains lock upon being called
    !C NOTE: Callers must keep track of the phys memory they used and free all of it. \
    The PMM cannot track `ORDER_BLOCK_SIZE` size allocated
*/
void freePhys(uint64_t ptr){
    if(IS_UNALIGNED(ptr)){kerror("Unaligned phys ptr (0x%lx)\n",ptr); return;}
    GET_LOCK(buddyLock);
    //Find region
    struct memRegion *reg = NULL;
    for(uint64_t i = 0; i < regionCount; ++i){
        if((region[i].base <= ptr)){}
        if((ptr < region[i].limit)){}
        if((ptr >= region[i].base) && (ptr <= region[i].limit)){
            //kinfo("Ptr[0x%lx] Base[0x%lx] Lim[0x%lx]\n",ptr,region[i].base,region[i].limit);
            reg = &region[i];
            break;
        }
    }
    if(!reg){kwarn("Invalid phys addr (0x%lx)\n",ptr); return;}
    //kinfo("Rel 0x%lx : Base 0x%lx\n",ptr-reg->base,reg->base);
    uint64_t bit = ORDER_ADDRESS_TO_BIT(ptr,reg->base);
    uint64_t idx = ORDER_GET_IDX_FROM_BIT(bit);
    //kinfo("Free'ing (0x%lx)->[O 0][I 0x%x][B 0x%x]\n",ptr,(uint32_t)idx,(uint32_t)bit);
    freeBit(reg,0,idx,bit);
    FREE_LOCK(buddyLock);
    return;
}

/*!
    !D Frees a physical address of `bytes` size
    !I ptr: Physical address to free
    !I bytes: Bytes to free
    !R None
    !C LOCK: This function does not obtain the lock. \
    freePhys() obtains the lock when called by this function.
    !C NOTE: Callers must keep track of the phys memory they used and free all of it. \
    The PMM cannot track `ORDER_BLOCK_SIZE` size allocated
*/
void freeNPhys(uint64_t ptr, size_t bytes){
    if(IS_UNALIGNED(ptr)){kerror("Unaligned phys ptr (0x%lx)\n",ptr); return;}
    for(uint32_t i = 0;; ++i){
        freePhys(ptr+(ORDER_MIN_BLOCK_SIZE*i));
        if((bytes-ORDER_MIN_BLOCK_SIZE) <= 0){return;}
        bytes -= ORDER_MIN_BLOCK_SIZE;
    }
}