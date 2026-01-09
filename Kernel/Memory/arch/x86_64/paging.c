#include <stdint.h>
#include <kprint.h>
#include <string.h>
#include <kcache.h>
#include <kernel.h>
#include "paging.h"
#include "pmm.h"
#include "vmm.h"
#include "kmalloc.h"
#include <dstack.h>
#include <task.h>

pageSpace kspace;
extern struct bootInfo *boot;
dstack *pageFrames = NULL;

/*!
    !D  Returns a copy of a requested page.
    !C NOTE: This function is primarily for the page fault handler
*/
uint64_t getPageInfo(uint64_t cr3, uint64_t virt, bool print){
    uint64_t *pml = (uint64_t*)cr3;
    if(print){knone("[PML][0x%x][0x%lx]\n\u21B3",PML_ENTRY(virt),cr3);}
    uint64_t *pdpe = (uint64_t*)TABLE_BASE(pml[PML_ENTRY(virt)]);
    if(print){
        if(pdpe){knone("[PDPE][0x%x][0x%lx]\n  \u21B3",PDPE_ENTRY(virt),(uint64_t)pdpe);}
        else{knone("[PDPE][No entry]\n");}
    }
    if(pdpe == NULL){return 0x0;}
    uint64_t *pde = (uint64_t*)TABLE_BASE(pdpe[PDPE_ENTRY(virt)]);
    if(print){
        if(pdpe){knone("[PDE][0x%x][0x%lx]\n    \u21B3",PDE_ENTRY(virt),(uint64_t)pde);}
        else{knone("[PDE][No entry]\n");}
    }
    if(pde == NULL){knone("[Invalid entry]\n"); return 0x0;}
    uint64_t *pte = (uint64_t*)TABLE_BASE(pde[PDE_ENTRY(virt)]);
    if(print){
        if(pdpe){
            knone("[PTE][0x%x][0x%lx]\n      \u21B3",PTE_ENTRY(virt),(uint64_t)pte);
            knone("(0x%lx)\n",(uint64_t)pte[PTE_ENTRY(virt)]);
        }
        else{knone("[PTE][No entry]\n");}
    }
    if(pte == NULL){return 0x0;}
    return pte[PTE_ENTRY(virt)];
}

void growPageStack(){
    kinfo("Adding to page frame data stack\n");
    size_t sz = 0x0;
    uint64_t phys = allocPhys(PAGE_SZ*100,MEMORY_TYPE_ANY,0,&sz);
    if(phys == MEMORY_ALLOCATION_FAILED){kpanic("Handle this error! (OOM?)",0);}
    uint64_t v = (uint64_t)vmallocPhys(phys,sz,VTYPE_DATA,VFLAG_MAKE|VFLAG_WRITE);
    if(!v){
        kpanic("Failed to allocated VMEM for page stack\n",0);
    }
    for(size_t i = 0; i <= sz; i+=PAGE_SZ){
        pushStack(pageFrames,phys+i);
    }
}

/*!
    Returns a pointer to a new pagetable from the data stack.
    Must be identity mapped and assumes a pagetable heap space is already mapped.
*/
uint64_t newPagetable(){
    if(stackCount(pageFrames) < 20){
        growPageStack();
    }
    start:
    bool fetched = false;
    uint64_t frame = popStack(pageFrames,&fetched);
    if(!fetched){
        growPageStack();
        goto start;
    }
    return frame;
}

/*!
    Maps a physical address to a virtual address.
    TODO: Return true on successful mapping
*/
bool mapPage(pageSpace *space, uint64_t phys, uint64_t virt, uint64_t flags){
    if(IS_UNALIGNED(phys) || IS_UNALIGNED(virt)){kwarn("Unaligned address\n"); return false;}
    //PML
    uint64_t pmln = PML_ENTRY(virt);
    bool make = flags&PAGE_FLAG_MAKE;
    if(TABLE_BASE(space->pml4e[pmln]) == 0x0){if(!make){return false;} space->pml4e[pmln] |= newPagetable();}
    uint64_t *pdpe = (uint64_t*)TABLE_BASE(space->pml4e[pmln]);
    //PDPE
    uint64_t pdpen = PDPE_ENTRY(virt);
    if(TABLE_BASE(pdpe[pdpen]) == 0x0){if(!make){return false;} pdpe[pdpen] |= newPagetable();}
    uint64_t *pde = (uint64_t*)TABLE_BASE(pdpe[pdpen]);
    //PDE
    uint64_t pden = PDE_ENTRY(virt);
    if(TABLE_BASE(pde[pden]) == 0x0){if(!make){return false;} pde[pden] |= newPagetable();}
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
    //kinfo("PTE 0x%lx PTEE 0x%x V 0x%lx\n",(uint64_t)pte,(uint32_t)PTE_ENTRY(virt),virt);
    if(PG_IS_PRESENT(pte[PTE_ENTRY(virt)]) != 0x0){invlpg(virt);}
    pte[PTE_ENTRY(virt)] = pgflags|phys;
    return true;
}

/*!
    Wrapper fof mapPage().
    Maps N number of pages.
*/
void mapPages(pageSpace *space, uint64_t phys, uint64_t virt, uint64_t flags, uint32_t pages){
    if((IS_UNALIGNED(phys)) || (IS_UNALIGNED(virt))){kerror("Unaligned addr (P0xl%x->V0x%lx)\n",phys,virt); return;}
    for(uint32_t b = 0; b < pages; ++b){
        bool r = mapPage(space,phys+(b*PAGE_SZ),virt+(PAGE_SZ*b),flags);
        if((flags&PAGE_FLAG_MAKE) != 0x0){KASSERT((r == true),"Failed to map page");}
    }
    //kinfo("Mapped [0x%x pages] 0x%lx->0x%lx (RANGE) 0x%lx->0x%lx\n",pages,phys,virt,virt,virt+(pages*PAGE_SZ));
}

/*!
    ASM wrapper for invalidating a page from the CPU cache.
*/
void invlpg(uint64_t addr){
    asm volatile("invlpg (%0)"::"r" (addr):);
}

/*!
        Unmaps a page.
        TODO: The code.
*/
void unmapPage(pageSpace *space, uint64_t virt){
    //
}

/*!
    Converts a virtual address to a physical address.
*/
uint64_t virtToPhysInt(pageSpace *space, uint64_t virt){
    uint64_t *pdpe = (uint64_t*)TABLE_BASE(space->pml4e[PML_ENTRY(virt)]);
    if(pdpe == NULL){return 0x0;}
    uint64_t *pde = (uint64_t*)TABLE_BASE(pdpe[PDPE_ENTRY(virt)]);
    if(pde == NULL){return 0x0;}
    uint64_t *pt = (uint64_t*)TABLE_BASE(pde[PDE_ENTRY(virt)]);
    if(pt == NULL){return 0x0;}
    return (uint64_t)TABLE_BASE(pt[PTE_ENTRY(virt)]);
}

/*!
    Changes the CPU's CR3 to the specificed pml4e.
*/
void setCR3(uint64_t cr3){
    asm volatile("movq %0, %%cr3" ::"a"(cr3));
}

/*!
    Intilizes paging.
*/
void initPaging(){
    //Set kpml
    uint64_t *kpml;
    asm volatile("movq %%cr3, %0":"=r" (kpml):);
    kinfo("CR3=0x%lx\n",(uint64_t)kpml);
    kspace.pml4e = kpml;
    pageFrames = createDstack(sizeof(uint64_t)*100);
    if(!pageFrames){kpanic("Failed to create dstack for page frame",0);}
    kinfo("Initilized paging\n");
}
