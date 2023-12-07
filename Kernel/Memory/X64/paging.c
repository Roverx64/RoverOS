#include <stdint.h>
#include <debug.h>
#include <string.h>
#include "paging.h"
#include "pmm.h"
#include "vmm.h"
#include "kheap.h"

pageSpace kspace;
extern struct bootInfo *boot;

//Returns a copy of a requested page
uint64 getPageInfo(uint64 cr3, uint64 virt){
    uint64 *pml = (uint64*)cr3;
    uint64 *pdpe = (uint64*)TABLE_BASE(pml[PML_ENTRY(virt)]);
    if(pdpe == NULL){return 0x0;}
    uint64 *pde = (uint64*)TABLE_BASE(pdpe[PDPE_ENTRY(virt)]);
    if(pde == NULL){return 0x0;}
    uint64 *pte = (uint64*)TABLE_BASE(pde[PDE_ENTRY(virt)]);
    if(pte == NULL){return 0x0;}
    return pte[PTE_ENTRY(virt)];
}

//Must be identity mapped and assumes heap space is already mapped
uint64 newPagetable(pageSpace *space){
    if(space->heapSize <= space->heapUsed){return 0x0;}
    space->heapUsed += 0x1000;
    uint64 ptr = (uint64)space->heapBase+space->heapUsed-0x1000;
    memset((void*)ptr,0x0,0x1000);
    return ptr;
}

void mapPage(pageSpace *space, uint64 phys, uint64 virt, bool write, bool user, bool execute, bool wc, bool make){
    if(IS_UNALIGNED(phys) || IS_UNALIGNED(virt)){kdebug(DWARN,"Unaligned address\n"); return;}
    //PML
    uint64 pmln = PML_ENTRY(virt);
    if(TABLE_BASE(space->pml4e[pmln]) == 0x0){if(!make){return;} space->pml4e[pmln] |= newPagetable(space);}
    uint64 *pdpe = (uint64*)TABLE_BASE(space->pml4e[pmln]);
    //PDPE
    uint64 pdpen = PDPE_ENTRY(virt);
    if(TABLE_BASE(pdpe[pdpen]) == 0x0){if(!make){return;} pdpe[pdpen] |= newPagetable(space);}
    uint64 *pde = (uint64*)TABLE_BASE(pdpe[pdpen]);
    //PDE
    uint64 pden = PDE_ENTRY(virt);
    if(TABLE_BASE(pde[pden]) == 0x0){if(!make){return;} pde[pden] |= newPagetable(space);}
    uint64 *pte = (uint64*)TABLE_BASE(pde[pden]);
    //Flags
    uint64 flags = PG_PRESENT;
    if(write){flags |= PG_WRITE;}
    space->pml4e[pmln] |= flags;
    pdpe[pdpen] |= flags;
    pde[pden] |= flags;
    if(user){flags |= PG_USER;}
    if(!execute){flags |= PG_NX;}
    if(wc){flags |= PG_WC;}
    if(PG_IS_PRESENT(pte[PTE_ENTRY(virt)]) != 0x0){invlpg(virt);}
    pte[PTE_ENTRY(virt)] = flags|phys;
}

void mapPages(pageSpace *space, uint64 phys, uint64 virt, bool write, bool user, bool execute, bool wc, bool make, uint32 pages){
    for(uint32 b = 0; b < pages; ++b){
        mapPage(space,phys+(b*PAGE_SZ),virt+(PAGE_SZ*b),write,user,execute,wc,make);
    }
}

void kmapPage(uint64 phys, uint64 virt, bool write, bool user, bool execute, bool wc, uint64 flagsin, bool make){
    mapPage(&kspace,phys,virt,write,user,execute,wc,make);
}

void kmapPages(uint64 phys, uint64 virt, bool write, bool user, bool execute, bool wc, bool make, uint32 pages){
    for(int i = 0; i < pages; ++i){
        mapPage(&kspace,phys+(i*PAGE_SZ),virt+(i*PAGE_SZ),write,user,execute,wc,make);
    }
}

void invlpg(uint64 addr){
    asm volatile("invlpg (%0)"::"r" (addr):);
}

void unmapPage(pageSpace *space, uint64 virt){
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

void setCR3(uint64 cr3){
    asm volatile("movq %0, %%cr3" ::"a"(cr3));
}

void initPaging(){
    //Set kpml
    uint64 *kpml;
    asm volatile("movq %%cr3, %0":"=r" (kpml):);
    kdebug(DINFO,"CR3=0x%lx\n",(uint64)kpml);
    kspace.pml4e = kpml;
    kspace.heapBase = boot->load.pageSlab+0x1000;
    kspace.heapSize = boot->load.slabSize-0x1000;
}