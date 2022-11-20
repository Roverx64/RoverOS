#include <efi.h>
#include <efilib.h>
#include "types.h"
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