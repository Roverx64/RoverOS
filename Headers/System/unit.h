#pragma once
#include "driver.h"
#include "mmap.h"
#include "mutex.h"

#define UNIT_LIBRARY_STATIC 1<<0 //Library is linked at a fixed address
#define UNIT_LIBRARY_PAGED 1<<1 //Library is swapped out to disk
#define UNIT_LIBRARY_NO_UNLOAD 1<<2 //Library may not be removed from memory

struct unitLibrary{
    mmapEntry *mmap;
    uint16_t flags;
    uint16_t references; //Number of processes using this library
};

extern bool isLibLoaded(char *lib);