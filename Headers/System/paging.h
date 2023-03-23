#pragma once
#include <stdint.h>

//2MB paging

#define CANONICAL_FORM 0x00000FFFFFFFFFFFF
#define PAGE_SZ 0x200000
#define PAGE_ALIGN 0xFFFFFFFFFFE00000 //2MB Align for phys pages
#define PG_STRUCT_ALIGN 0x1000 //4KB alignment

#define ENTRY_MASK 0x1FF
#define PAGE_MASK 0xFFFFFFFF000
#define GET_PDE_ENTRY(v) (((v&PAGE_MASK)>>21)&ENTRY_MASK)
#define GET_PDPE_ENTRY(v) (((v&PAGE_MASK)>>30)&ENTRY_MASK)
#define GET_PML_ENTRY(v) (((v&PAGE_MASK)>>39)&ENTRY_MASK)

#define GET_ADDR_BASE(addr) (((uint64)addr&PAGE_MASK)>>12)
#define GET_PHYS_BASE(addr) (((uint64)addr&PAGE_ALIGN)>>21)
#define BASE_TO_PTR(base) ((void*)((uint64)base<<12))
#define KALIGN(addr) ((uint64)addr&PAGE_ALIGN)
#define IS_ALIGNED(addr) ((uint64)addr&0x1FFFFF) //Works like strcmp

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
    uint8 a1 : 1;
    uint8 global : 1;
    uint8 avl : 3;
    uint8 pat : 1;
    uint8 mbz : 8;
    uint64 addr : 31; //Ptr to phys addr
    uint16 avl1 : 11;
    uint8 nx : 1;
}__attribute__((packed));