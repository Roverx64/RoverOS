#pragma once

#include "types.h"

#define ROVEROS_BASE_LOAD 0x10000 //Phys memory load address
#define ROVEROS_BASE_VLOAD 0xC0000000 //Virtal mapping start address

struct bootInfoMmap{
    uintptr *mmap; //Pointer to memory map
    uint64 mmapSize; //Size of memory map
    uint64 mmapKey;
    uint64 mmapDescriptorSize;
    uintptr baseLoad;
};

struct bootInfoACPI{
    uint64 xsdt;
};

//For use in the kernel
struct bootInfoS{
    struct bootInfoMmap memory;
    struct bootInfoACPI acpi;
};

struct bootInfoS bootInfo;