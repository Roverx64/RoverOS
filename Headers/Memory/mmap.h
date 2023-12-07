#pragma once

//Passed to the kernel from the UEFI bootloader

#define MMAP_TYPE_FREE 0x1
#define MMAP_TYPE_UEFI 0x2
#define MMAP_TYPE_VESA 0x3
#define MMAP_TYPE_MMIO 0x4
#define MMAP_TYPE_ACPI 0x5
#define MMAP_TYPE_RESV 0x6
#define MMAP_TYPE_UNUSABLE 0x7
#define MMAP_MAGIC 0xAEFADEAD
#define MMAP_ATTRIBUTE_KERNEL 1<<0

typedef struct mmapEntry{
    uint32 magic;
    uint16 type;
    uint16 attributes;
    uint64 phys; //Should generally be identity mapped
    uint64 virt;
    uint32 bytes;
}mmapEntry;