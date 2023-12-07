#pragma once
#include <stdint.h>
#include "vmm.h"

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

#define PG_IS_PRESENT(pg) (pg&0x1)
#define PG_IS_WRITEABLE(pg) ((pg>>1)&0x1)
#define PG_IS_USER(pg) ((pg>>2)&0x1)
#define PG_IS_DIRTY(pg) ((pg>>6)&0x1)
#define PG_IS_ACCESSED(pg) ((pg>>5)&0x1)
#define PG_IS_WC(pg) ((pg>>12)&0x1)
#define PG_IS_NX(pg) (pg>>63)

#define CANONICAL_FORM 0x00000FFFFFFFFFFFF
#define PAGE_SZ 0x1000 //4KB pages
#define PAGE_ALIGN 0xFFFFFFFFFFFFF000
#define SELECT_MASK 0x1FF
#define BASE_MASK 0x000FFFFFFFFFF000

#define PML_ENTRY(vaddr) ((vaddr>>39)&SELECT_MASK)
#define PDPE_ENTRY(vaddr) ((vaddr>>30)&SELECT_MASK)
#define PDE_ENTRY(vaddr) ((vaddr>>21)&SELECT_MASK)
#define PTE_ENTRY(vaddr) ((vaddr>>12)&SELECT_MASK)

#define TABLE_BASE(entry) ((entry)&BASE_MASK)

#define KALIGN(addr) ((uint64)addr&PAGE_ALIGN)
#define IS_UNALIGNED(addr) ((uint64)addr&0xFFF)

extern uint64 getPageInfo(uint64 cr3, uint64 virt);
extern void mapPage(pageSpace *space, uint64 phys, uint64 virt, bool write, bool user, bool execute, bool wc, bool make);
extern void kmapPage(uint64 phys, uint64 virt, bool write, bool user, bool execute, bool wc, uint64 flagsin, bool make);
extern void *kallocatePages(uint64 phys, bool write, bool user, bool execute, bool wc, uint32 pages, uint64 flags);
extern void mapPages(pageSpace *space, uint64 phys, uint64 virt, bool write, bool user, bool execute, bool wc, bool make, uint32 pages);
extern void setCR3(uint64 cr3);
extern void copyKpages(pageSpace *dest, uint64 virt, bool make, uint32 pages);
extern void dumpVPath(pageSpace *space, uint64 virt);
extern void unmapPage(pageSpace *space, uint64 virt);
extern void invlpg(uint64 addr);