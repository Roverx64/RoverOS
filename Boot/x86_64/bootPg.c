#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "bootMmap.h"
#include "bootPaging.h"

struct pml4e *pml = NULL;
void *pageBase = NULL;
uint64 baseOffset = 0x0;

struct pdpe *newPdpe(){
    void *ptr = (void*)((uint64)pageBase+baseOffset);
    baseOffset += (sizeof(struct pdpe)*512);
    return (struct pdpe*)ptr;
}

struct pde *newPde(){
    void *ptr = (void*)((uint64)pageBase+baseOffset);
    baseOffset += (sizeof(struct pde)*512);
    return (struct pde*)ptr;
}

void mapPage(uint64 phys, uint64 virt, bool w, bool u, bool nx){
    //Ensure addr is aligned
    uint64 alignedPhys = KALIGN(phys);
    uint64 alignedVirt = KALIGN(virt);
    uint32 pdeEntry = GET_PDE_ENTRY(alignedVirt);
    uint32 pdpeEntry = GET_PDPE_ENTRY(alignedVirt);
    uint32 pmlEntry = GET_PML_ENTRY(alignedVirt);
    //Set pml vars
    pml[pmlEntry].present = true;
    pml[pmlEntry].write = true;
    pml[pmlEntry].user = false;
    pml[pmlEntry].nx = false;
    if(pml[pmlEntry].addr == 0x0){pml[pmlEntry].addr = GET_ADDR_BASE(newPdpe());}
    struct pdpe *pdpe = (struct pdpe*)BASE_TO_PTR(pml[pmlEntry].addr);
    pdpe[pdpeEntry].present = true;
    pdpe[pdpeEntry].write = true;
    pdpe[pdpeEntry].user = false;
    pdpe[pdpeEntry].nx = false;
    if(pdpe[pdpeEntry].addr == 0x0){pdpe[pdpeEntry].addr = GET_ADDR_BASE(newPde());}
    struct pde *pde = (struct pde*)BASE_TO_PTR(pdpe[pdpeEntry].addr);
    pde[pdeEntry].present = true;
    pde[pdeEntry].a1 = 1;
    //Page was set to write, so leave it set to avoid a UEFI page fault
    //The C kernel will fix this later (Read NOTE below)
    if(!pde[pdeEntry].write){pde[pdeEntry].write = w;}
    pde[pdeEntry].user = u;
    //NOTE: All memory will be executable
    //The kernel will move the UEFI code/data to a 2MB boundary later and use NX appropriately
    //pde[pdeEntry].nx = nx;
    pde[pdeEntry].addr = GET_PHYS_BASE(alignedPhys);
}

void mapPages(uint64 phys, uint64 virt, uint32 pages, bool w, bool u, bool nx){
    if(pages == 0x0){pages = 1;}
    /*Print(L"Mapping 0x%llx pages 0x%llx->0x%llx [",pages,KALIGN(phys),KALIGN(virt));
    if(w){Print(L"W");}else{Print(L"-");}
    if(u){Print(L"U");}else{Print(L"K");}
    if(nx){Print(L"-");}else{Print(L"X");}
    Print(L"]\n");*/
    for(int i = 0; i < pages; ++i){
        mapPage(phys,virt,w,u,nx);
        virt += PAGE_SZ;
        phys += PAGE_SZ;
    }
}

EFI_STATUS initPaging(){
    uint64 cr3 = 0x0;
    asm volatile("movq %%cr3, %0" : "=r"(cr3));
    Print(L"Current CR3: 0x%llx\n",cr3);
    pageBase = getMemory(PAGE_SZ*4); //2MB should be enough
    if(pageBase == NULL){return !EFI_SUCCESS;}
    //Align base
    uint64 mapBase = (uint64)pageBase;
    pageBase = (void*)(KALIGN(pageBase)+PAGE_SZ);
    Print(L"Allocated memory at 0x%llx\n",(uint64)pageBase);
    //Set pml
    pml = (struct pml4e*)pageBase;
    //Identity map self
    baseOffset += sizeof(struct pml4e)*512;
    mapPages((uint64)pageBase,(uint64)pageBase,2,true,false,true);
    Print(L"Identity mapped pml4e\n");
    mapMMAP();
    Print(L"Identity mapped MMAP\n");
    //Map framebuffer
    mapPages((uint64)kinf.ui.framebuffer,(uint64)kinf.ui.framebuffer,kinf.ui.sz/PAGE_SZ,true,false,true);
    Print(L"Mapped framebuffer\n");
    return EFI_SUCCESS;
}

//Sets new page map
void setCR3(){
    asm volatile("movq %%rax, %%cr3" ::"a"(pml));
}

//For debugging

uint32 pmln = 0x0;
uint32 pdpen = 0x0;


void dumpPDE(uint64 ptr){
    if(ptr == 0x0){return;}
    struct pde *pde = (struct pde*)BASE_TO_PTR(ptr);
    uint64 virt = 0x0;
    virt = (virt|pmln)<<9;
    virt = (virt|pdpen)<<9;
    for(uint32 i = 0; i < 512; ++i){
        if(!pde[i].present){continue;}
        virt = (virt|i)<<21;
        Print(L"    [PDE-0x%lx|0x%llx->0x%llx|",i,(uint64)(pde[i].addr<<21),virt);
        virt = (virt>>30);
        virt = (virt<<9);
        if(pde[i].present){Print(L"P");}else{Print(L"-");}
        if(pde[i].write){Print(L"W");}else{Print(L"-");}
        if(pde[i].user){Print(L"U");}else{Print(L"K");}
        if(pde[i].nx){Print(L"-");}else{Print(L"X");}
        Print(L"]\n");
    }
}

void dumpPDPE(uint64 ptr){
    if(ptr == 0x0){return;}
    struct pdpe *pdpe = (struct pdpe*)BASE_TO_PTR(ptr);
    for(uint32 i = 0; i < 512; ++i){
        if(!pdpe[i].present){continue;}
        pdpen = i;
        Print(L"  [PDPE-0x%lx|0x%llx|",i,(uint64)BASE_TO_PTR(pdpe[i].addr));
        if(pdpe[i].present){Print(L"P");}else{Print(L"-");}
        if(pdpe[i].write){Print(L"W");}else{Print(L"-");}
        if(pdpe[i].user){Print(L"U");}else{Print(L"K");}
        if(pdpe[i].nx){Print(L"-");}else{Print(L"X");}
        Print(L"]\n");
        dumpPDE(pdpe[i].addr);
    }
}

void dumpPMAP(bool efi){
    struct pml4e *pmle;
    uint64 cr3 = 0x0;
    if(efi){
        asm volatile("movq %%cr3, %0" : "=r"(cr3));
        pmle = (struct pml4e*)cr3;
    }
    else{
        cr3 = GET_ADDR_BASE(pml)<<12;
        pmle = pml;
    }
    Print(L"CR3: 0x%llx\n",cr3);
    for(uint32 i = 0; i < 512; ++i){
        if(!pmle[i].present){continue;}
        pmln = i;
        Print(L"[PML-0x%lx|0x%llx|",i,(uint64)BASE_TO_PTR(pml[i].addr));
        if(pmle[i].present){Print(L"P");}else{Print(L"-");}
        if(pmle[i].write){Print(L"W");}else{Print(L"-");}
        if(pmle[i].user){Print(L"U");}else{Print(L"K");}
        if(pmle[i].nx){Print(L"-");}else{Print(L"X");}
        Print(L"]\n");
        //Print pdpe
        dumpPDPE(pmle[i].addr);
    }
}
