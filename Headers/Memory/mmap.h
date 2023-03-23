#pragma once

//Passed to the kernel from the UEFI bootloader

#define MMAP_TYPE_FREE 0x1
#define MMAP_TYPE_UEFI 0x2
#define MMAP_TYPE_VESA 0x3
#define MMAP_TYPE_MMIO 0x4
#define MMAP_TYPE_ACPI 0x5
#define MMAP_TYPE_RESV 0x6
#define MMAP_TYPE_UNUSABLE 0x7


typedef struct mmapEntry{
    uint16 type;
    uint64 phys; //Should generally be identity mapped
    uint64 virt;
    uint16 pages;
}mmapEntry;