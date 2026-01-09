#pragma once

#include "stdint.h"

//Passed to the kernel from the UEFI bootloader

#define MMAP_TYPE_IGNORE 0x0 //Invalid entry that must be ignored by the parser
#define MMAP_TYPE_FREE 0x1
#define MMAP_TYPE_UEFI 0x2
#define MMAP_TYPE_VESA 0x3
#define MMAP_TYPE_MMIO 0x4
#define MMAP_TYPE_ACPI 0x5
#define MMAP_TYPE_RESV 0x6
#define MMAP_TYPE_UNUSABLE 0x7
#define MMAP_TYPE_KERNEL 0x8 //!< Kernel code/data
#define MMAP_TYPE_OTHER 0x9 //!< Misc data
#define MMAP_TYPE_ELF 0xA //!< ELF data
#define MMAP_TYPE_PAGE 0xB //!< Paging data
#define MMAP_TYPE_BOOT 0xC //!< Bootloader data
#define MMAP_TYPE_PMM 0xD //PMM chunk region
#define MMAP_TYPE_MAX 0xD
#define MMAP_MAGIC 0xAEFADEADAEFADEAD

#define MMAP_FLAG_ALIGN 1<<0 //!< Specifies to add EFI_PAGE_SZ to ptr
#define MMAP_FLAG_PARSED 1<<1 //!< Specifies that the PMM has already created a region for this mmap entry

typedef struct mmapEntry{
    union{
        uint64_t magic;
        uint64_t offset; //Only used in the ARM bootloader stage
    };
    uint8_t flags;
    uint64_t type;
    uint64_t phys;
    uint64_t virt; //Ignored for libraries
    uint64_t bytes;
}mmapEntry;


//The bootloader will not have these functions
#ifndef BOOTLOADER_COMPILE_STAGE //This macro must be added with gcc's -D option

extern void initMmap(struct bootInfo *kinf);
extern mmapEntry *getMmap();
extern char *mmapTypeString[MMAP_TYPE_MAX+1];
extern void dumpMmap();
extern mmapEntry *getMmapEntry_internal(uint64_t phys,bool alloc);

#define getMmapEntry(phys) getMmapEntry_internal(phys,false)
#define getAllocMapEntry(phys) getMmapEntry_internal(phys,true)

#endif