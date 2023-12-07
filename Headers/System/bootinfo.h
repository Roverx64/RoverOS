#pragma once

//Using stdint instead of boottypes causes compiling issues
//So this will work well enough
typedef unsigned int bi32;
typedef unsigned long bi64;

#define BOOTINFO_MAGIC 0x424F4F54494E464F //BOOTINFO

//Passed to the kernel

struct gopInfo{
    //Screen info
    bi32 Xres;
    bi32 Yres;
    bi32 pps; //Pixels per scanline
    //Masks
    bi32 rmask;
    bi32 gmask;
    bi32 bmask;
    bi32 resmask;
    //Buffer
    bi64 sz;
    void *framebuffer;
    //Console font
    void *font;
};

struct loadInfo{
    bi64 phys; //Where the kernel is in phys memory
    bi64 virt; //What addr phys is mapped to
    bi64 size; //Size of the kernel
    bi64 rdptr; //Ramdisk pointer
    bi64 kheapBase; //Physical base address
    bi64 kheapVBase; //Virtual base address
    bi64 kheapSize; //Heap size
    bi64 pageSlab; //Identity mapped slab
    bi64 slabSize;  //Size of slab
    bi64 endOfKernel; //End of kernel in VRAM
};

struct memInfo{
    void *ptr;
    bi32 entries;
    bi64 totalMem;
    bi64 usableMem;
};

struct modInfo{
    void *modList;
    bi64 mods;
};

struct bootInfo{
    unsigned long long magic;
    struct gopInfo ui;
    struct loadInfo load;
    struct memInfo mem;
    struct modInfo mod;
    bi64 *testELF;
    bi64 *xsdtptr;
};