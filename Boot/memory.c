#include <efi.h>
#include <efilib.h>
#include "types.h"
#include "pml4e.h"
#include "boot.h"
#include "functions.h"

EFI_STATUS getMmap(struct bootInfoS *bootInfo,uint64 ksz){
    Print(L"Obtaining mmap\n");
    UINTN mmapDescVer = 0;
    bootInfo->memory.mmap = (uintptr*)LibMemoryMap((UINTN*)&bootInfo->memory.mmapSize,(UINTN*)&bootInfo->memory.mmapKey,(UINTN*)&bootInfo->memory.mmapDescriptorSize,(UINT32*)&mmapDescVer);
    Print(L"Obtained mmap version 0x%lx at 0x%llx\n",mmapDescVer,(uint64)bootInfo->memory.mmap);
    EFI_MEMORY_DESCRIPTOR *entry = (EFI_MEMORY_DESCRIPTOR*)bootInfo->memory.mmap;
    while((uint64)entry < (uint64)bootInfo->memory.mmap+(bootInfo->memory.mmapSize*bootInfo->memory.mmapDescriptorSize)){
        /*0 will be replaced with the RoverOS.bin size soon*/
        if(entry->Type == EfiConventionalMemory && GET_ENTRY_SZ(entry->NumberOfPages,EFI_PAGE_SIZE) >= ksz){
            bootInfo->memory.baseLoad = entry->PhysicalStart;
            bootInfo->memory.baseVload = ROVEROS_BASE_LOAD;
            Print(L"Using 0x%llx->0x%llx for phys | 0x%llx for virt | Size: 0x%llx\n",(uint64)entry->PhysicalStart,(uint64)entry->PhysicalStart+(uint64)GET_ENTRY_SZ(entry->NumberOfPages,EFI_PAGE_SIZE),(uint64)bootInfo->memory.baseVload,(uint64)GET_ENTRY_SZ(entry->NumberOfPages,EFI_PAGE_SIZE));
            break;
        }
        entry = NextMemoryDescriptor(entry,bootInfo->memory.mmapDescriptorSize);
    }
    //Check if a valid entry was found
    if(bootInfo->memory.baseLoad == 0x0){Print(L"No suitable memory found\n"); return !EFI_SUCCESS;}
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