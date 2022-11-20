#pragma once

#include "types.h"

#define ROVEROS_BASE_LOAD 0xC0000000 //Virtal mapping start address

struct bootInfoMmap{
    uintptr *mmap; //Pointer to memory map
    uint64 mmapSize; //Size of memory map
    uint64 mmapKey;
    uint64 mmapDescriptorSize;
    //Kernel
    uintptr baseLoad;
    uintptr baseVload;
};

struct bootInfoACPI{
    uint64 xsdt;
};

struct bootInfoIO{
    uintptr idtp;
    uintptr idt;
};

//For use in the kernel
struct bootInfoS{
    struct bootInfoMmap memory;
    struct bootInfoACPI acpi;
    struct bootInfoIO io;
};

/*=====Macros=====*/
//Mmap
#define GET_ENTRY_SZ(pages,pgsz) ((uint64)pages*pgsz)