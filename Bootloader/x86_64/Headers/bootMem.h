#pragma once
#include <sefi.h>
#include <stdint.h>
#include "mmap.h"

#define MEM_DEBUG 0 //Set to 0 to disable debugging

extern void *allocMem_int(const char *func, uint64_t ln, size_t sz, uint64_t virt, uint8_t type, bool temp, bool align, bool clear);

#define NEXT_DESC(ptr,sz) ((EFI_MEMORY_DESCRIPTOR*)((uint64_t)ptr+sz))
#define DESC_SZ(desc) (desc->NumberOfPages*EFI_PAGE_SIZE)
#define allocMem(sz,virt,type,temp,align,clear) allocMem_int(__FUNCTION__,__LINE__,sz,virt,type,temp,align,clear)

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
#define PG_WC 1<<12

#define CANONICAL_FORM 0x00000FFFFFFFFFFFF
#define PAGE_SZ 0x1000 //4KB pages
#define PAGE_ALIGN 0xFFFFFFFFFFFFF000
#define PAGE_LOW_MASK 0x1FFFFF
#define SELECT_MASK 0x1FF
#define BASE_MASK 0x000FFFFFFFFFF000

#define PML_ENTRY(vaddr) ((vaddr>>39)&SELECT_MASK)
#define PDPE_ENTRY(vaddr) ((vaddr>>30)&SELECT_MASK)
#define PDE_ENTRY(vaddr) ((vaddr>>21)&SELECT_MASK)
#define PTE_ENTRY(vaddr) ((vaddr>>12)&SELECT_MASK)

#define TABLE_BASE(entry) ((entry)&BASE_MASK)

#define KALIGN(addr) ((uint64_t)addr&PAGE_ALIGN)
#define IS_ALIGNED(addr) ((uint64_t)addr&0x1FFFFF) //Works like strcmp

extern void initMem();
extern EFI_STATUS initMMAP();
extern EFI_STATUS mapPages(uint64_t phys, uint64_t virt, uint32_t n, bool write, bool execute);
extern EFI_STATUS mapPage(uint64_t phys, uint64_t virt, bool write, bool execute);
extern void testPath(uint64_t addr, bool efi);