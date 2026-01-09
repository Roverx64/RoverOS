#include <kcache.h>
#include <stdint.h>
#include <string.h>
#include <hashtable.h>
#include <pmm.h>
#include <paging.h>
#include <kmalloc.h>
#include <kprint.h>
#include <mutex.h>
#include <cpu.h>
#include <kernel.h>
#include <pointer.h>

kcache internalkcacheCache; //Cache of 'kcache' objects
char *kcName = "kcache_cache";
kslab *freeList = NULL;
kslab *inUse = NULL;
mutex_t listLock = 0;

//NOTE: Very unfinished SLAB code
//Many hacks were done here

void freeSlab(kslab *slab){
}

/*!
    !D Carves up a new slab for a cache
    NOTE: Assumes addr is mapped as rw
    Caller must have the cache lock
*/
kslab *carveSlab(kcache *cache, uint64_t virt, uint64_t phys, size_t slabsz){
    kslab *slab = (kslab*)kmalloc(sizeof(kslab));
    if(!slab){kerror("Failed to allocate slab\n"); return NULL;}
    slab->flags = 0x0;
    slab->references = 0;
    slab->size = slabsz;
    slab->phys = phys;
    slab->virt = virt;
    slab->prev = NULL;
    slab->next = NULL;
    bool redzone = cache->flags&KCACHE_FLAG_REDZONE;
    slab->bufferCount = slab->size/cache->objSize;
    //Probably shouldn't use the kernel heap, but whatever for now
    uint64_t n = slab->bufferCount/64;
    if(!n){n = 1;}
    slab->bitmap = (uint64_t*)kmalloc(sizeof(uint64_t)*n);
    memset(slab->bitmap,0x0,sizeof(uint64_t)*n);
    FREE_LOCK(slab->lock);
    return slab;
}

/*!
    Caller must have the cache lock
*/
kslab *newSlabPhys(kcache *cache, uint64_t phys, size_t physSz){
    //Allocate virt memory
    uint64_t virt = 0x0;
    if((cache->flags&KCACHE_FLAG_NO_MAP) == 0){
        virt = (uint64_t)vmallocPhys(phys,physSz,VTYPE_HEAP,VFLAG_MAKE|VFLAG_WRITE);
        if(!virt){kerror("valloc() failed\n"); return NULL;}
    }
    kslab *slab = carveSlab(cache,virt,phys,physSz);
    if(!slab){vmFree((void*)virt); return NULL;}
    return slab;
}

/*!
    Creates a new slab with 'count' number of objects.
    It's not guarunteed a slab will return with 'count' number of slots availible.
    Caller must have the cache lock
*/
kslab *newSlab(kcache *cache, size_t count){
    size_t sz = 0x0;
    //kinfo("0x%lx\n",sz);
    //Allocate phys memory
    uint64_t phys = allocPhys(cache->objSize*count,cache->type,(cache->type == MEMORY_TYPE_ANY) ? 0 : ORDER_FLAG_STRICT_MATCH,&sz);
    if(phys == MEMORY_ALLOCATION_FAILED){kwarn("Failed to allocate phys memory. OOM?\n"); return NULL;}
    kslab *nslb = newSlabPhys(cache,phys,sz);
    if(!nslb){freeNPhys(phys,sz);}
    return nslb;
}

//Cache functions

kcacheConstructor *slabConstructor(){

}

kcacheDestructor *slabDestructor(){

}

/*!
    Useful for statically allocated caches (Eg. 'internalkcacheCache)
*/
void fillCacheDetails(kcache *cache, char *name, size_t objSize, uint16_t align, uint8_t type, uint64_t flags, kcacheConstructor *constructor, kcacheDestructor *destructor){
    cache->flags = flags&KCACHE_VALID_FLAGS;
    cache->name = (char*)kmalloc((strlen(name)+1)*sizeof(char));
    if(!cache->name){kerror("kmalloc() failed. using 'name' ptr instead."); cache->name = name;}
    else{strcpy(cache->name,name);}
    cache->slabs = NULL; //Created on allocation
    cache->objSize = objSize;
    cache->constructor = (constructor) ? constructor : slabConstructor;
    cache->destructor = (destructor) ? destructor : slabDestructor;
    cache->type = type;
    FREE_LOCK(cache->lock);
}

//Gets free buffer from slab and returns a pointer
//Ignores alignment for now
void *allocateBuffer(kcache *cache, kslab *slab){
    GET_LOCK(slab->lock);
    if(slab->bufferCount == slab->references){kinfo("SLAB is full!\n"); FREE_LOCK(slab->lock); return NULL;}
    //kinfo("Checking for free buffer {0x%lx R 0x%lx C}\n",slab->references,slab->bufferCount);
    uint32_t left = slab->bufferCount;
    bool found = false;
    uint32_t i = 0;
    uint32_t b = 0;
    uint32_t checked = 0;
    //kinfo("0x%x buffers to test\n",slab->bufferCount);
    while(checked < slab->bufferCount){
        i = checked/64;
        ++checked;
        if(b == 64){b = 0;}
        //kinfo("Testing [0x%x][0x%x]\n",i,b);
        if(slab->bitmap[i]&(0x1<<b)){++b; continue;}
        found = true;
        break;
    }
    exit:
    if(!found){FREE_LOCK(slab->lock); kinfo("No free bit found\n"); return NULL;} //Extend heap
    //Calculate address
    uint64_t ptr = (i*64)+b;
    ptr *= cache->objSize;
    ptr += slab->virt;
    slab->bitmap[i] |= (0x1<<b);
    slab->references += 1;
    FREE_LOCK(slab->lock);
    return (void*)ptr;
}

void *kcacheAlloc(kcache *cache, uint64_t flags){
    GET_LOCK(cache->lock);
    kslab *chosen = cache->slabs;
    while(chosen != NULL){
        if(chosen->bufferCount > chosen->references){break;}
        chosen = (kslab*)chosen->next;
    }
    if(!chosen){
        chosen = newSlab(cache,10);
        if(!chosen){
            kerror("Failed to allocate new memory\n");
            if(cache->flags&KCACHE_FLAG_NO_WAIT){FREE_LOCK(cache->lock); return NULL;}
            /*Yield time slice and try again here*/
            FREE_LOCK(cache->lock);
            return NULL;
        }
        chosen->next = cache->slabs;
        cache->slabs = (void*)chosen;
        if(chosen->next){((kslab*)chosen->next)->prev = (void*)chosen;}
    }
    void *ptr = allocateBuffer(cache,chosen);
    FREE_LOCK(cache->lock);
    return ptr;
}

void kcacheFree(kcache *cache, void *ptr){
}

/*!
    Creates a new cache.
*/
kcache *kcacheCreate(char *name, size_t objSize, uint16_t align, uint8_t type, uint64_t flags, kcacheConstructor *constructor, kcacheDestructor *destructor){
    kcache *cache = kcacheAlloc(&internalkcacheCache,0);
    if(!cache){kerror("Failed to allocated from internal cache\n"); return NULL;}
    fillCacheDetails(cache,name,objSize,align,type,flags,constructor,destructor);
    kinfo("Created new cache '%s' [Sz 0x%lx|A 0x%x|F 0x%lx]\n",name,objSize,align,flags);
    return cache;
}

void kcacheDestroy(kcache *cache){
}

void kcacheGrow(kcache *cache){
    kslab *slab = newSlab(cache,10);
    KASSERT((slab != NULL),"NULL slab");
    slab->next = (void*)cache->slabs;
    cache->slabs = slab;
    kinfo("Extended kcache '%s'\n",cache->name);
}

void kcacheReap(kcache *cache){
}

void initSLAB(struct bootInfo *kinf){
    kinfo("Initilizing SLAB allocator\n");
    fillCacheDetails(&internalkcacheCache,&kcName,sizeof(kcache),0,MEMORY_TYPE_ANY,0,NULL,NULL);
    kinfo("Initilized SLAB\n");
}

kcache *createVMarkerCache(char *name, size_t objSize, uint16_t align, uint8_t type, uint64_t flags, kcacheConstructor *constructor, kcacheDestructor *destructor){
    kcache *cache = (kcache*)kmalloc(sizeof(kcache));
    if(!cache){kpanic("Failed to allocate VMM cache",0);}
    fillCacheDetails(cache,name,objSize,align,type,flags,constructor,destructor);
    void *ptr = kmallocAligned(PAGE_SZ); //The OS will panic should this alloc fail
    cache->slabs = carveSlab(cache,(uint64_t)ptr,0x0,PAGE_SZ);
    KASSERT((cache->slabs != NULL),"Failed to carve slab");
    kinfo("Created vMarker cache '%s' [Sz 0x%lx|A 0x%x|F 0x%lx]\n",name,objSize,align,flags);
    return cache;
}