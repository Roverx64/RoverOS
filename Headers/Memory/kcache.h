#pragma once

#include <stdint.h>
#include <hashmap.h>

#define KCACHE_VALID_FLAGS 0x0F //Valid flags mask. Anything outside of this is invalid
#define KCACHE_FLAG_REDZONE 1<<0 //Enables redzone guard words at the end of each buffer
#define KCACHE_FLAG_COLORING 1<<1 //Enable slab coloring for better CPU cache utilization
#define KCACHE_FLAG_NO_WAIT 1<<2 //Tells the allocator to skip allocation if no memory is available
#define KSLAB_FLAG_NO_REAP 1<<3 //Disallow this slab from being destroyed
#define KSLAB_FLAG_RECENT 1<<4 //Slab was recently use and not eligible to be reaped

typedef void (*kcacheConstructor)(void*,size_t);
typedef void (*kcacheDestructor)(void*,size_t);

typedef struct{
    bool inUse;
    void *ptr;
    void *slab;
}kbuff;

typedef struct{
    uint16_t references; //How many buffers are in use
    size_t size;
    uint16_t bufferCount;
    kbuff *buff;
    void *prev;
    void *next;
}kslab;

typedef struct{
    uint64_t flags; //Cache flags
    char *name; //Human readable name
    size_t objSize;
    uint16_t align; //Byte alignment; 0 = no alignment
    uint8_t type; //1MB,ISA,DMA,Low,High,etc
    kslab *slabs;
    hashmap *map;
    kcacheConstructor *constructor;
    kcacheDestructor *destructor;
}kcache;

extern kcache *kcacheCreate(char *name, size_t objSize, uint16_t align, uint64_t flags, kcacheConstructor *constructor, kcacheDestructor *destructor);
extern void kcacheDestroy(kcache *cache);
extern void *kcacheAlloc(kcache *cache, uint64_t flags);
extern void kcacheFree(kcache *cache, void *ptr);
extern void kcacheGrow(kcache *cache);
extern void kcacheReap(kcache *cache);