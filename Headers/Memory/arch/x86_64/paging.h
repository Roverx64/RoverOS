#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <vmm.h>

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
#define PG_NX (uint64_t)1<<63
#define PG_UC PG_PWT|PG_PAT|PG_PCD
#define PG_WC PG_PWT
#define PG_WB PG_PAT
#define PG_WP 0 //This type is not currently in the PAT
#define PG_WT PG_PAT|PG_PWT

#define PG_IS_PRESENT(pg) (pg&0x1)
#define PG_IS_WRITEABLE(pg) ((pg>>1)&0x1)
#define PG_IS_USER(pg) ((pg>>2)&0x1)
#define PG_IS_DIRTY(pg) ((pg>>6)&0x1)
#define PG_IS_ACCESSED(pg) ((pg>>5)&0x1)
#define PG_IS_WC(pg) ((pg>>12)&0x1)
#define PG_IS_NX(pg) (pg>>63)

#define PAGE_LEVEL 4
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

#define KALIGN(addr) ((uint64_t)addr&PAGE_ALIGN)
#define IS_UNALIGNED(addr) ((uint64_t)addr&0xFFF)

#define PAGE_FLAG_UC 1<<0
#define PAGE_FLAG_WC 1<<1 //NOTE: WC is set to the PA1 register. Only PWT bit needs to be set to use WC
#define PAGE_FLAG_WB 1<<2
#define PAGE_FLAG_WP 1<<3
#define PAGE_FLAG_WT 1<<4
#define PAGE_FLAG_WRITE 1<<5
#define PAGE_FLAG_USER 1<<6
#define PAGE_FLAG_EXECUTE 1<<7
#define PAGE_FLAG_MAKE 1<<8

extern uint64_t getPageInfo(uint64_t cr3, uint64_t virt, bool print);
extern bool mapPage(pageSpace *space, uint64_t phys, uint64_t virt, uint64_t flags);
extern void mapPages(pageSpace *space, uint64_t phys, uint64_t virt, uint64_t flags, uint32_t pages);
extern void setCR3(uint64_t cr3);
extern void unmapPage(pageSpace *space, uint64_t virt);
extern void invlpg(uint64_t addr);
extern uint64_t virtToPhys(uint64_t virt);