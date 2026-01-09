#pragma once

#include <stdint.h>
#include <hashtable.h>
#include <mutex.h>

#define KCACHE_VALID_FLAGS 0x0F //Valid flags mask. Anything outside of this is invalid
#define KCACHE_FLAG_REDZONE 1<<0 //Enables redzone guard qwords at the end of each buffer
#define KCACHE_FLAG_COLORING 1<<1 //Enable slab coloring for better CPU cache utilization
#define KCACHE_FLAG_NO_WAIT 1<<2 //Tells the allocator to skip allocation if no memory is available
#define KCACHE_FLAG_NO_MAP 1<<3 //Don't map the physical slabs to vmem

#define KSLAB_VALID_FLAGS 0x03
#define KSLAB_FLAG_NO_REAP 1<<0 //Disallow this slab from being destroyed
#define KSLAB_FLAG_RECENT 1<<1 //Slab was recently used and not eligible to be reaped

#define KSLAB_NEXT_BUFFER(phys,align,trail) ((phys&((~(align>>trail))<<trail))+align-1)
#define KSLAB_ALIGN_POINTER(ptr,align) (ptr+(align-(ptr%align)))
#define KSLAB_IS_UNALIGNED(phys,align,trail) (phys&(~((~(align>>trail))<<trail)))
#define KSLAB_BUFFER_OFFSET(maxptr,buffers) (maxptr-(sizeof(uint8_t)*buffers))
#define KSLAB_CALCULATE_PTR(base,pad,objSz,absBit) (base+((objSz+pad)*absBit))
#define KSLAB_REDZONE_MAGIC 0xDEFEDEABEAFFCAFE

#define KBUFF_FLAG_FREE 1<<0

typedef void (*kcacheConstructor)(void*,size_t);
typedef void (*kcacheDestructor)(void*,size_t);


typedef struct{
    uint8_t flags;
    uint64_t start; //Start of buffer in memory
}kbuff;

typedef struct{
    uint32_t flags;
    uint16_t references; //How many buffers are in use
    size_t size; //Order size
    uint32_t bufferCount; //Total number of buffers
    uint64_t phys; //Physical address
    uint64_t virt; //Virtual address
    void *prev; //Prev slab
    void *next; //Next slab
    uint64_t *bitmap; //Bitmap of buffers
    mutex_t lock;
}kslab;

typedef struct{
    uint64_t flags; //Cache flags
    char *name; //Human readable name
    size_t objSize;
    uint16_t align; //Byte alignment; 0 = no alignment
    uint8_t type; //1MB,ISA,DMA,Low,High,etc
    union{
        kslab *slabs; //Main slab objects
        hashtable *pageTable; //Less complex for allocating memory for page tables
    };
    kcacheConstructor *constructor;
    kcacheDestructor *destructor;
    mutex_t lock;
}kcache;

extern kcache *kcacheCreate(char *name, size_t objSize, uint16_t align, uint8_t type, uint64_t flags, kcacheConstructor *constructor, kcacheDestructor *destructor);
extern void kcacheDestroy(kcache *cache);
extern void *kcacheAlloc(kcache *cache, uint64_t flags);
extern void kcacheFree(kcache *cache, void *ptr);
extern void kcacheGrow(kcache *cache);
extern void kcacheReap(kcache *cache);