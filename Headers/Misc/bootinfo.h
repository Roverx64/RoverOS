#pragma once
#include <stdint.h>

#define BOOTINFO_MAGIC 0x424F4F54494E464F //BOOTINFO

//Passed to the kernel

struct gopInfo{
    //Screen info
    uint32_t Xres;
    uint32_t Yres;
    uint32_t pps; //Pixels per scanline
    //Masks
    uint32_t rmask;
    uint32_t gmask;
    uint32_t bmask;
    uint32_t resmask;
    //Buffer
    uint64_t sz;
    void *framebuffer;
    //Console font
    void *font;
};

struct memInfo{
    void *ptr; //Pointer to mmap
    void *sptr; //Pointer to allocation mmap
    uint32_t entries; //mmap entries
    uint64_t totalMem; //Total, non mmio, ram available to use
    uint64_t usableMem; //Total usable ram available
    uint64_t chunkSize; //Size of chunk
    uint64_t chunkPtr; //Phys pointer to PMM's chunk of memory. This will be identity mapped
};

struct bootInfo{
    unsigned long long magic;
    struct gopInfo ui;
    struct memInfo mem;
    uint64_t rdptr; //Ramdisk pointer
    uint64_t xsdtptr; //Pointer to the XSDT/RSDT
};