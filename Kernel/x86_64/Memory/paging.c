#include <stdint.h>
#include "paging.h"
#include "pmm.h"

struct pml4e *kpml;

//Returns a copy of a requested page
//If pde doesn't exist, then a1 will be set to 0
void getPageInfo(pageSpace *space, void *dest, uint64 virt){
    if(space == NULL || dest == NULL){return;}
    uint64 *pdeDest = (uint64*)dest;
    virt = KALIGN(virt);
    uint32 pmlEntry = GET_PML_ENTRY(virt);
    uint32 pdpeEntry = GET_PDPE_ENTRY(virt);
    uint32 pdeEntry = GET_PDE_ENTRY(virt);
    if(space->pml[pmlEntry].addr == 0x0){goto nexist;}
    struct pdpe *pdpe = (struct pdpe*)BASE_TO_PTR(space->pml[pmlEntry].addr);
    if(pdpe[pdpeEntry].addr == 0x0){goto nexist;}
    uint64 *pde = (uint64*)BASE_TO_PTR(pdpe[pdpeEntry].addr);
    //Every entry is 8 bytes long
    pdeDest[0] = pde[pdeEntry];
    return;
    nexist:
    ((struct pde*)pdeDest)->a1 = 0;
    return;
}

void mapPage(uint64 phys, uint64 virt, bool make, bool write, bool user, bool nx){
    phys = KALIGN(phys);
    virt = KALIGN(virt);
    uint32 pmlEntry = GET_PML_ENTRY(virt);
    uint32 pdpeEntry = GET_PDPE_ENTRY(virt);
    uint32 pdeEntry = GET_PDE_ENTRY(virt);
    if(kpml[pmlEntry].addr == 0x0 && !make){return;}
    struct pdpe *pdpe = (struct pdpe*)BASE_TO_PTR(kpml[pmlEntry].addr);
    if(pdpe[pdpeEntry].addr == 0x0 && !make){return;}
    //Set PML
    kpml[pmlEntry].present = true;
    kpml[pmlEntry].write = true;
    kpml[pmlEntry].user = false;
    kpml[pmlEntry].nx = false;
    if(kpml[pmlEntry].addr == 0x0){/*Allocate pdpe*/}
    //Set PDPE
    pdpe[pdpeEntry].present = true;
    pdpe[pdpeEntry].write = true;
    pdpe[pdpeEntry].user = false;
    pdpe[pdpeEntry].nx = false;
    if(pdpe[pdpeEntry].addr == 0x0){/*Allocate pde*/}
    struct pde *pde = (struct pde*)BASE_TO_PTR(pdpe[pdpeEntry].addr);
    //Set PDE
    pde[pdeEntry].present = true;
    pde[pdeEntry].write = write;
    pde[pdeEntry].user = user;
    pde[pdeEntry].nx = nx;
    pde[pdeEntry].a1 = 1; //PS Bit
    pde[pdeEntry].addr = GET_PHYS_BASE(phys);
}

void unmapPage(uint64 virt){
    virt = KALIGN(virt);
    uint32 pmlEntry = GET_PML_ENTRY(virt);
    uint32 pdpeEntry = GET_PDPE_ENTRY(virt);
    uint32 pdeEntry = GET_PDE_ENTRY(virt);
    if(kpml[pmlEntry].addr == 0x0){return;}
    struct pdpe *pdpe = (struct pdpe*)BASE_TO_PTR(kpml[pmlEntry].addr);
    if(pdpe[pdpeEntry].addr == 0x0){return;}
    struct pde* pde = (struct pde*)BASE_TO_PTR(pdpe[pmlEntry].addr);
    pde[pdeEntry].present = false;
}