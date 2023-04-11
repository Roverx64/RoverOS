#include <stdint.h>
#include "malloc.h"
#include "palloc.h"
#include "paging.h"

//Very early malloc
//Nowhere near the final version

void *malloc(size sz){
    //2MB at a time for now
    //Identity mapped for now
    uint64 phys = (uint64)palloc();
    mapPage(phys,phys,true,true,false,true);
    return (void*)phys;
}

void free(void *ptr){
    pfree(ptr);
    unmapPage((uint64)ptr);
}