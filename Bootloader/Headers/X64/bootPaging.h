#pragma once
#include <efi.h>
#include "boottypes.h"

//2MB paging

#define PG_PRESENT 1<<0
#define PG_WRITE 1<<1
#define PG_USER 1<<2
#define PG_PWT 1<<3
#define PG_PCD 1<<4
#define PG_ACCESSED 1<<5
#define PG_DIRTY 1<<6
#define PG_2M 1<<7
#define PG_GLOBAL 1<<8
#define PG_PAT 1<<12
#define PG_NX (uint64)1<<63
#define PG_WC 1<<12

#define CANONICAL_FORM 0x00000FFFFFFFFFFFF
#define PAGE_SZ 0x1000 //4KB pages
#define PAGE_ALIGN 0xFFFFFFFFFFFFF000
#define PAGE_LOW_MASK 0x1FFFFF
#define SELECT_MASK 0x1FF
#define BASE_MASK 0x000FFFFFFFFFF000

//000FFFFFFFFFF000

#define PML_ENTRY(vaddr) ((vaddr>>39)&SELECT_MASK)
#define PDPE_ENTRY(vaddr) ((vaddr>>30)&SELECT_MASK)
#define PDE_ENTRY(vaddr) ((vaddr>>21)&SELECT_MASK)
#define PTE_ENTRY(vaddr) ((vaddr>>12)&SELECT_MASK)

#define TABLE_BASE(entry) ((entry)&BASE_MASK)

#define KALIGN(addr) ((uint64)addr&PAGE_ALIGN)
#define IS_ALIGNED(addr) ((uint64)addr&0x1FFFFF) //Works like strcmp

extern EFI_STATUS initPaging();
extern EFI_STATUS mapPages(uint64 phys, uint64 virt, uint32 n, bool write, bool user, bool execute);
extern EFI_STATUS mapPage(uint64 phys, uint64 virt, bool write, bool user, bool execute);

struct pml4e{
    uint8 present : 1;
    uint8 write : 1;
    uint8 user : 1;
    uint8 pwt : 1;
    uint8 pcd : 1;
    uint8 accessed : 1;
    uint8 ign : 1;
    uint8 mbz : 2;
    uint8 avl : 3; //Free for kernel use
    uint64 addr : 40; //Ptr to pdpe
    uint16 avl1 : 11;
    uint8 nx : 1;
}__attribute__((packed));

struct pdpe{
    uint8 present : 1;
    uint8 write : 1;
    uint8 user : 1;
    uint8 pwt : 1;
    uint8 pcd : 1;
    uint8 accessed : 1;
    uint8 ign : 1;
    uint8 mbz : 1;
    uint8 a0 : 1;
    uint8 avl : 3;
    uint64 addr : 40; //Ptr to pde
    uint16 avl1 : 11;
    uint8 nx : 1;
}__attribute__((packed));

struct pde{
    uint8 present : 1;
    uint8 write : 1;
    uint8 user : 1;
    uint8 pwt : 1;
    uint8 pcd : 1;
    uint8 accessed : 1;
    uint8 dirty : 1;
    uint8 ps : 1;
    uint8 global : 1;
    uint8 avl : 3;
    uint8 pat : 1;
    uint8 mbz : 8;
    uint64 addr : 31; //Ptr to phys addr
    uint16 avl1 : 11;
    uint8 nx : 1;
}__attribute__((packed));