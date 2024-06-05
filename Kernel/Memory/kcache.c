#include <kcache.h>
#include <stdint.h>
#include <string.h>
#include <hashmap.h>
#include <pmm.h>
#include <paging.h>
#include <kmalloc.h>
#include <debug.h>

kcache slabCache; //Internal cache

uint64_t kcacheHasher(uint64_t key){
    return key;
}

kcacheConstructor slabConstructor(){
    return 0;
}

kcacheDestructor slabDestructor(){
    return 0;
}

extern void *pmmBumpAlloc(struct bootInfo *kinf,size_t sz);

void initSLAB(struct bootInfo *kinf){
    kdebug(DINFO,"Initilizing SLAB allocator\n");
    slabCache.flags = 0x0;
    slabCache.name = (char*)kmalloc(sizeof(char)*16);
    strcpy(slabCache.name,(char*)"kcache_slab\0");
    slabCache.objSize = sizeof(kcache);
    slabCache.align = 0;
    slabCache.type = MEMORY_TYPE_ANY;
    slabCache.slabs = NULL;
    slabCache.map = NULL;
    slabCache.constructor = slabConstructor;
    slabCache.destructor = slabDestructor;
    kdebug(DINFO,"Initilized SLAB\n");
}

kcache *kcacheCreate(char *name, size_t objSize, uint16_t align, uint64_t flags, kcacheConstructor *constructor, kcacheDestructor *destructor){
    //Check if flags are valid
    if(flags > KCACHE_VALID_FLAGS){return NULL;}
    //Allocate space for the cache and name using the slower kmalloc
    kcache *cache = (kcache*)kmalloc(sizeof(kcache)+strlen(name));
    cache->name = (char*)((uint64_t)cache+sizeof(kcache));
    strcpy(cache->name,name);
    //cache->slabs = newSlab(objSize,align,MEMORY_TYPE_ANY,flags&KCACHE_FLAG_REDZONE);
    cache->slabs->prev = NULL;
    cache->slabs->next = NULL;
    cache->constructor = constructor;
    cache->destructor = destructor;
    //cache->map = hashmapCreate();
    return cache;
}

void kcacheDestroy(kcache *cache){
    //Free all slabs
    kcacheReap(cache);
}

//Gets free buffer from slab and returns a pointer
void *allocateBuffer(kcache *cache, kslab *slab){
    for(uint16_t i = 0; i < slab->bufferCount; ++i){
        if(slab->buff[i].inUse){continue;}
        slab->buff[i].inUse = true;
        slab->references += 1;
        hashmapInsert(cache->map,&slab->buff[i].ptr,&slab->buff[i],false);
        return slab->buff[i].ptr;
    }
    //We shouldn't end up here
    return NULL;
}

void *kcacheAlloc(kcache *cache, uint64_t flags){
    //Check slabs for free buffers
    kslab *slab = cache->slabs;
    while(slab->next != NULL){
        if(slab->references == slab->bufferCount){continue;}
        return allocateBuffer(cache,slab);
    }
    kcacheGrow(cache);
    //Return buffer from new slab
    slab = (kslab*)cache->slabs->next;
    return allocateBuffer(cache,slab);
}

void kcacheFree(kcache *cache, void *ptr){
    //kbuff *buff = (kbuff*)hashmapGetValue(cache->map,(uint64_t)ptr);
    //if(buff == NULL){return;}
    //buff->inUse = false;
    //buff->slab->references -= 1;
}

//Carves up a new slab for a cache
kslab *newSlab(size_t objSz, uint16_t align, uint8_t type, bool redZone){
    kslab *slab = (kslab*)kmalloc(sizeof(kslab));
    uint64_t order = (objSz/PAGE_SZ);
    uint64_t phys = allocPhys(order,type,MEMORY_TYPE_ANY);
    uint64_t sz = (order*PAGE_SZ)+PAGE_SZ;
    //Determine number of buffers needed
    uint64_t buffers = sz/objSz;
    uint64_t extra = sz-(buffers*objSz);
    uint64_t zones = buffers*sizeof(uint64_t);
    if(redZone){}
    slab->size = objSz;
    slab->references = 0;
    slab->buff = NULL;
    slab->next = NULL;
    slab->prev = NULL;
    return slab;
}

void kcacheGrow(kcache *cache){
    kslab *slab = newSlab(cache->objSize,cache->align,cache->type,cache->flags&KCACHE_FLAG_REDZONE);
    if(slab == NULL){return;}
    slab->next = cache->slabs->next;
    slab->prev = cache->slabs;
    cache->slabs->next = (void*)slab;
    ((kslab*)slab->next)->prev = (void*)slab;
}

void kcacheReap(kcache *cache){
}
