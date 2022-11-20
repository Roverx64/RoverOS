#include <efi.h>
#include <efilib.h>
#include "types.h"
#include "interrupt.h"
#include "pml4e.h"
#include "boot.h"
#include "functions.h"

struct vspace kspace;

irh pageFault(){
    uint64 cr2;
    asm volatile("movq %%cr2, %0" : "=r"(cr2));
    Print(L"Page fault[0x%llx]\n",cr2);
    for(;;){}
}

void initPaging(){
    uint64 cr3;
    asm("mov %%cr3, %0":"=r"(cr3));
    Print(L"CR3 at 0x%llx\n",(uint64)cr3);
    kspace.pml = (struct pml4e*)cr3;
}

#define GET_PDE(v) (v>>21)&0x1FF
#define GET_PDPE(v) (v>>30)&0x1FF
#define GET_PML(v) (v>>39)&0x1FF
#define GET_BASE(v) (v)>>EFI_PAGE_SHIFT

//For debugging
void readAddr(uintptr virt){
    uint16 pml = GET_PML(virt);
    uint16 pdpe = GET_PDPE(virt);
    uint16 pde = GET_PDE(virt);
    //PML
    Print(L"|[0x%x|0x%llx=0x%llx|",(int)pml,(uint64)kspace.pml,(uint64)kspace.pml);
    if(kspace.pml[0].entry[pml].present == 1){Print(L"P");}else{Print(L"A");}
    if(kspace.pml[0].entry[pml].write == 1){Print(L"W");}else{Print(L"R");}
    if(kspace.pml[0].entry[pml].user == 1){Print(L"U");}else{Print(L"K");}
    if(kspace.pml[0].entry[pml].nx == 1){Print(L"-");}else{Print(L"X");}
    Print(L"]\n+->");
    //PDPE
    kspace.pdpe = (struct pdpe*)((uintptr)kspace.pml[0].entry[pml].base<<EFI_PAGE_SHIFT);
    Print(L"[0x%x|0x%llx=0x%llx|",(int)pdpe,(uint64)kspace.pml[0].entry[pml].base,(uint64)((uintptr)kspace.pml[0].entry[pml].base<<EFI_PAGE_SHIFT));
    if(kspace.pdpe[pml].entry[pdpe].present == 1){Print(L"P");}else{Print(L"A");}
    if(kspace.pdpe[pml].entry[pdpe].write == 1){Print(L"W");}else{Print(L"R");}
    if(kspace.pdpe[pml].entry[pdpe].user == 1){Print(L"U");}else{Print(L"K");}
    if(kspace.pdpe[pml].entry[pdpe].nx == 1){Print(L"-");}else{Print(L"X");}
    Print(L"]\n+-->");
    //PDE
    kspace.pde = (struct pde*)((uintptr)kspace.pdpe[pml].entry[pdpe].base<<EFI_PAGE_SHIFT);
    Print(L"[0x%x|0x%llx=0x%llx|",(int)pde,(uint64)kspace.pdpe[pml].entry[pdpe].base,(uint64)((uintptr)kspace.pdpe[pml].entry[pdpe].base<<EFI_PAGE_SHIFT));
    if(kspace.pde[pdpe].entry[pde].present == 1){Print(L"P");}else{Print(L"A");}
    if(kspace.pde[pdpe].entry[pde].write == 1){Print(L"W");}else{Print(L"R");}
    if(kspace.pde[pdpe].entry[pde].user == 1){Print(L"U");}else{Print(L"K");}
    if(kspace.pde[pdpe].entry[pde].nx == 1){Print(L"-");}else{Print(L"X");}
    kspace.pde[pdpe].entry[pde].nx = 0x1;
    Print(L"]\n");
    //kspace.pde[pdpe].entry[pde].nx = 0;
    Print(L"+--->[0x%llx=0x%llx]\n",kspace.pde[pdpe].entry[pde].base,(uint64)((uint64)kspace.pde[pdpe].entry[pde].base<<EFI_PAGE_SHIFT));
}

EFI_STATUS mapPage(uintptr phys, uintptr virt, bool write, bool user, bool nx, bool alloc){
    if(phys&EFI_PAGE_MASK){phys = phys^EFI_PAGE_MASK;}
    if(virt&EFI_PAGE_MASK){virt = virt^EFI_PAGE_MASK;}
    Print(L"[0x%llx->0x%llx|",phys,virt);
    if(write){Print(L"W");}else{Print(L"R");}
    if(user){Print(L"U");}else{Print(L"K");}
    if(nx){Print(L"-");}else{Print(L"X");}
    if(alloc){Print(L"A");}else{Print(L"-");}
    Print(L"]\n");
    readAddr(virt);
    return !EFI_SUCCESS;
}