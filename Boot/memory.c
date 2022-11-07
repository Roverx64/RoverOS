#include <efi.h>
#include <efilib.h>
#include "types.h"
#include "pml4e.h"
#include "boot.h"
#include "functions.h"

EFI_MEMORY_DESCRIPTOR *start = NULL;
uint32 entries;
uint64 descSize = 0;

void *getSection(uint64 size, uint64 attributes){ //Returns first available phys addr for use
    EFI_MEMORY_DESCRIPTOR *entry = start;
    void *ret = (void*)GS_INVALID_PTR;
    uint32 i = entries;
    while(i > 0){
        if((entry->Attribute&attributes) == attributes && entry->Type == EfiConventionalMemory && GET_ENTRY_SZ(entry->NumberOfPages,EFI_PAGE_SIZE) >= size){
            ret = (void*)entry->PhysicalStart;
            break;
        }
        entry = NextMemoryDescriptor(entry,descSize);
        --i;
    }
    return ret;
}

EFI_STATUS initMmap(struct bootInfoS *bootInfo){
    Print(L"Initilizing mmap\n");
    //Get memory map from UEFI
    UINTN mmapDescVer = 0;
    bootInfo->memory.mmap = (uintptr*)LibMemoryMap((UINTN*)&bootInfo->memory.mmapSize,(UINTN*)&bootInfo->memory.mmapKey,(UINTN*)&bootInfo->memory.mmapDescriptorSize,(UINT32*)&mmapDescVer);
    //Set vars
    start = (EFI_MEMORY_DESCRIPTOR*)bootInfo->memory.mmap;
    entries = bootInfo->memory.mmapSize;
    descSize = bootInfo->memory.mmapDescriptorSize;
    Print(L"Obtained mmap version 0x%lx at 0x%llx with 0x%llx entries\n",(uint32)mmapDescVer,(uint64)bootInfo->memory.mmap,(uint64)entries);
    return EFI_SUCCESS;
}

struct vspace kspace;

EFI_STATUS mapMemory(struct bootInfoS *bootInfo){
    //Get UEFI's pml4e
    uint64 cr3;
    asm("mov %%cr3, %0":"=r"(cr3));
    Print(L"CR3 at 0x%llx\n",(uint64)cr3);
    kspace.pml = (struct pml4e*)cr3;
    //Get vars
    uint64 addr = bootInfo->memory.baseVload;
    uint16 pmlN = GET_PML(addr);
    uint16 pdpeN = GET_PDPE(addr);
    uint16 pdeN = GET_PDE(addr);
    //Setup structure
    if(kspace.pml->entry[pmlN].base == 0x0){
        cr3 = (uint64)AllocatePool(sizeof(struct pdpe));
        if(cr3 == 0x0){Print(L"Failed to allocate pdpe\n");return !EFI_SUCCESS;}
        kspace.pml->entry[pmlN].present = true;
        kspace.pml->entry[pmlN].base = (cr3&EFI_PAGE_MASK)>>EFI_PAGE_SHIFT;
    }
    return EFI_SUCCESS;
}