#pragma once
#include <stdint.h>

#define BOOTINFO_MAGIC 0x424F4F54494E464F //BOOTINFO

struct gopInfo{
    uint32_t Xres; //X resolution
    uint32_t Yres; //Y resolution
    uint32_t pps; //Pixels per scanline
    uint32_t rmask; //Red mask
    uint32_t gmask; //Green mask
    uint32_t bmask; //Blue mask
    uint32_t resmask; //Reserved mask
    uint64_t size; //Size of buffer
    uint64_t framebuffer; //Pointer to framebuffer
};

struct memInfo{
    uint64_t ptr; //Pointer to mmap array
    uint32_t entries; //Number of entries in array
    uint64_t aptr; //!< Pointer to allocation map
    uint32_t aentries; //!< Number of entries in allocation map
    uint64_t symptr; //!< Pointer to symbol file
    uint64_t symsz; //!< Size of symbol file
};

struct bootInfo{
    uint64_t magic; //Must be 0x424F4F54494E464F (BOOTINFO)
    struct gopInfo ui; //UI information structure
    struct memInfo mem; //Memory information structure
    uint64_t rdptr; //Ramdisk pointer (0x0 if no ramdisk is loaded)
    uint64_t rdsz; //Ramdisk size
    uint64_t acpiptr; //Pointer to the XSDP/RSDP
    void *devtree; //Points to a parsed device tree. NULL on x86_64
};