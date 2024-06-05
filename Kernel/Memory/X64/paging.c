#include <stdint.h>
#include <debug.h>
#include <string.h>
#include "paging.h"
#include "pmm.h"
#include "vmm.h"
#include "kmalloc.h"

pageSpace kspace;
extern struct bootInfo *boot;

//Returns a copy of a requested page
uint64_t getPageInfo(uint64_t cr3, uint64_t virt){
    uint64_t *pml = (uint64_t*)cr3;
    uint64_t *pdpe = (uint64_t*)TABLE_BASE(pml[PML_ENTRY(virt)]);
    if(pdpe == NULL){return 0x0;}
    uint64_t *pde = (uint64_t*)TABLE_BASE(pdpe[PDPE_ENTRY(virt)]);
    if(pde == NULL){return 0x0;}
    uint64_t *pte = (uint64_t*)TABLE_BASE(pde[PDE_ENTRY(virt)]);
    if(pte == NULL){return 0x0;}
    return pte[PTE_ENTRY(virt)];
}

//Must be identity mapped and assumes a pagetable heap space is already mapped
uint64_t newPagetable(pageSpace *space){
    uint64_t ptr = space->pageHeap+space->ptr;
    memset((void*)ptr,0x0,0x1000);
    space->ptr += 0x1000;
    kdebug(DINFO,"Returning 0x%lx\n",ptr);
    return ptr;
}

void mapPage(pageSpace *space, uint64_t phys, uint64_t virt, uint64_t flags){
    if(IS_UNALIGNED(phys) || IS_UNALIGNED(virt)){kdebug(DWARN,"Unaligned address\n"); return;}
    //PML
    uint64_t pmln = PML_ENTRY(virt);
    bool make = flags&PAGE_FLAG_MAKE;
    if(TABLE_BASE(space->pml4e[pmln]) == 0x0){if(!make){return;} space->pml4e[pmln] |= newPagetable(space);}
    uint64_t *pdpe = (uint64_t*)TABLE_BASE(space->pml4e[pmln]);
    //PDPE
    uint64_t pdpen = PDPE_ENTRY(virt);
    if(TABLE_BASE(pdpe[pdpen]) == 0x0){if(!make){return;} pdpe[pdpen] |= newPagetable(space);}
    uint64_t *pde = (uint64_t*)TABLE_BASE(pdpe[pdpen]);
    //PDE
    uint64_t pden = PDE_ENTRY(virt);
    if(TABLE_BASE(pde[pden]) == 0x0){if(!make){return;} pde[pden] |= newPagetable(space);}
    uint64_t *pte = (uint64_t*)TABLE_BASE(pde[pden]);
    //Flags
    uint64_t pgflags = PG_PRESENT;
    if(flags&PAGE_FLAG_WRITE){pgflags |= PG_WRITE;}
    space->pml4e[pmln] |= pgflags;
    pdpe[pdpen] |= pgflags;
    pde[pden] |= pgflags;
    if(flags&PAGE_FLAG_USER){pgflags |= PG_USER;}
    if(!(flags&PAGE_FLAG_EXECUTE)){pgflags |= PG_NX;}
    if((flags&0x1F) != 0x0){
        if(flags&PAGE_FLAG_UC){pgflags |= PG_UC; goto end;}
        if(flags&PAGE_FLAG_WC){pgflags |= PG_WC; goto end;}
        if(flags&PAGE_FLAG_WB){pgflags |= PG_WB; goto end;}
        if(flags&PAGE_FLAG_WP){pgflags |= PG_WP; goto end;}
        if(flags&PAGE_FLAG_WT){pgflags |= PG_WT; goto end;}
    }
    end:
    if(PG_IS_PRESENT(pte[PTE_ENTRY(virt)]) != 0x0){invlpg(virt);}
    pte[PTE_ENTRY(virt)] = pgflags|phys;
}

void mapPages(pageSpace *space, uint64_t phys, uint64_t virt, uint64_t flags, uint32_t pages){
    for(uint32_t b = 0; b < pages; ++b){
        mapPage(space,phys+(b*PAGE_SZ),virt+(PAGE_SZ*b),flags);
    }
}

void kmapPage(uint64_t phys, uint64_t virt, uint64_t flags){
    mapPage(&kspace,phys,virt,flags);
}

void kmapPages(uint64_t phys, uint64_t virt, uint64_t flags, uint32_t pages){
    for(uint32_t i = 0; i < pages; ++i){
        mapPage(&kspace,phys+(i*PAGE_SZ),virt+(i*PAGE_SZ),flags);
    }
}

void invlpg(uint64_t addr){
    asm volatile("invlpg (%0)"::"r" (addr):);
}

void unmapPage(pageSpace *space, uint64_t virt){
    //
}

uint64_t virtToPhys(pageSpace *space, uint64_t virt){
    uint64_t *pdpe = (uint64_t*)TABLE_BASE(space->pml4e[PML_ENTRY(virt)]);
    if(pdpe == NULL){return 0x0;}
    uint64_t *pde = (uint64_t*)TABLE_BASE(pdpe[PDPE_ENTRY(virt)]);
    if(pde == NULL){return 0x0;}
    uint64_t *pt = (uint64_t*)TABLE_BASE(pde[PDE_ENTRY(virt)]);
    if(pt == NULL){return 0x0;}
    return (uint64_t)TABLE_BASE(pt[PTE_ENTRY(virt)]);
}

void setCR3(uint64_t cr3){
    asm volatile("movq %0, %%cr3" ::"a"(cr3));
}

void initPaging(){
    //Set kpml
    uint64_t *kpml;
    asm volatile("movq %%cr3, %0":"=r" (kpml):);
    kdebug(DINFO,"CR3=0x%lx\n",(uint64_t)kpml);
    kspace.pml4e = kpml;
    kspace.ptr = 0xAEFFFFAE000; //Ptr for testing
    //Bootstrap kernel's table space
    
}

//1000000000000000000000000000000000000000000000000000000000001001