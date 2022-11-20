#pragma once

#include "types.h"

/*PML4E paging structure as defined by AMD64*/
/*Using 4 Level paging*/

struct pml4eEntry{
    bool present : 1;
    bool write : 1;
    bool user : 1;
    bool pwt : 1;
    bool pcd : 1;
    bool accessed : 1;
    uint8 ign0 : 3;
    uint8 avl0 : 3;
    uint64 base : 40;
    uint16 avl1 : 11;
    bool nx : 1;
}__attribute__((packed));

struct pdpeEntry{
    bool present : 1;
    bool write : 1;
    bool user : 1;
    bool pwt : 1;
    bool pcd : 1;
    bool accessed : 1;
    bool ign0 : 1;
    bool a0 : 1;
    bool ign1 : 1;
    uint8 avl0 : 3;
    uint64 base : 40;
    uint16 avl1 : 11;
    bool nx : 1;
}__attribute__((packed));

struct  pdeEntry{
    bool present : 1;
    bool write : 1;
    bool user : 1;
    bool pwt : 1;
    bool pcd : 1;
    bool accessed : 1;
    bool dirty : 1;
    bool a1 : 1;
    bool global : 1;
    uint8 avl0 : 3;
    bool pat : 1;
    uint8 ign1 : 8;
    uint32 base : 31;
    uint16 avl : 11;
    bool nx : 1;
}__attribute__((packed));

struct pml4e{
    struct pml4eEntry entry[512];
}__attribute__((packed));

struct pdpe{
    struct pdpeEntry entry[512];
}__attribute__((packed));

struct pde{
    struct pdeEntry entry[512];
}__attribute__((packed));

struct vspace{
    struct pml4e *pml;
    struct pdpe *pdpe;
    struct pde *pde;
}__attribute__((packed));