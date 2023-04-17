#pragma once

//Using stdint of boottypes causes compiling issues
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
};

struct loadInfo{
    bi64 phys; //Where the kernel is in phys memory
    bi64 virt; //What addr phys is mapped to
    bi64 size; //Size of the kernel
    bi64 rdptr;
    bi64 stackTop;
};

struct memInfo{
    void *ptr;
    bi32 entries;
    bi64 totalMem;
    bi64 usableMem;
};

struct bootInfo{
    unsigned long long magic;
    struct gopInfo ui;
    struct loadInfo load;
    struct memInfo mem;
    bi64 *xsdtptr;
};