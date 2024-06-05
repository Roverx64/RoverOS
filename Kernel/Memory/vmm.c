#include <stdint.h>
#include <pmm.h>
#include <debug.h>
#include <paging.h>
#include <vmm.h>

//Simple bump pointer for now

uint64_t valloc(pageSpace *space, size_t bytes, uint64_t phys, uint64_t flags){
    uint64_t virt = space->ptr;
    uint64_t pages = bytes/PAGE_SZ;
    mapPages(space,phys,virt,flags|PAGE_FLAG_MAKE,bytes/PAGE_SZ);
    space->ptr += (pages*PAGE_SZ);
    return virt;
}

void vfree(pageSpace *space, uint64_t virt){
    unmapPage(space,virt);
}