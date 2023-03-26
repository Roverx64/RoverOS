#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "bootMmap.h"
#include "bootPaging.h"
#include "mmap.h"

EFI_MEMORY_DESCRIPTOR *descStart = NULL;
UINTN descEntries = 0;
UINTN descKey = 0;
UINTN descSz = 0;
UINT32 descVer = 0;
void *kmmapBase = NULL;

void *getMemory(uint64 size){
    uint64 ptr = 0x0;
    //Fails on real hardware for some reason
    EFI_STATUS status = uefi_call_wrapper(BS->AllocatePages,4,AllocateAnyPages,EfiLoaderData,size/EFI_PAGE_SIZE,&ptr);
    if(status == EFI_OUT_OF_RESOURCES){goto retry;}
    if(status != EFI_SUCCESS){goto retry;}
    goto end;
    retry:
    ptr = (uint64)AllocatePool(size); //Better than nothing
    if(ptr == 0x0){
        Print(L"Failed to find 0x%llx free bytes\n",size);
        return NULL;
    }
    end:
    SetMem((void*)ptr,size,0x0);
    return (void*)ptr;
}

EFI_STATUS getMmap(){
    EFI_STATUS status = EFI_SUCCESS;
    descStart = LibMemoryMap(&descEntries,&descKey,&descSz,&descVer);
    if((EFI_STATUS)descStart == EFI_INVALID_PARAMETER){Print(L"MapKey was invalid\n"); return status;}
    Print(L"Got MMAP [0x%llx|0x%llx entries|0x%llx dSz|0x%lx Ver]\n",(uint64)descStart,descEntries,descSz,descVer);
    kmmapBase = getMemory(sizeof(mmapEntry)*descEntries);
    if(kmmapBase == NULL){Print(L"Failed to find space for kmmap\n"); return !EFI_SUCCESS;}
    kinf.mem.ptr = kmmapBase;
    return EFI_SUCCESS;
}

void updateMmap(){
    descStart = LibMemoryMap(&descEntries,&descKey,&descSz,&descVer);
}

//Maps all entries in the mmap
void mapMMAP(){
    updateMmap(); //Ensure latest version before mapping
    EFI_MEMORY_DESCRIPTOR *descriptor = descStart;
    mmapEntry *entry = (mmapEntry*)kmmapBase;
    bool write = false;
    bool user = false;
    bool nx = false;
    bool map = false;
    uint16 mType = 0x0;
    uint64 sz = 0x0;
    uint64 usable = 0x0;
    uint16 entryn;
    for(int i = 0; i < descEntries; ++i){
        map = false;
        sz += descriptor->NumberOfPages*EFI_PAGE_SIZE;
        ++entryn;
        //Print(L"0x%llx->0x%llx 0x%llx pgs\n",(uint64)descriptor->PhysicalStart,(uint64)descriptor->VirtualStart,(uint64)descriptor->NumberOfPages);
        switch(descriptor->Type){
            case EfiMemoryMappedIOPortSpace:
            case EfiMemoryMappedIO:
            mType = MMAP_TYPE_MMIO;
            break;
            case EfiBootServicesCode:
            map = true;
            write = false;
            user = false;
            nx = false;
            mType = MMAP_TYPE_FREE;
            usable += descriptor->NumberOfPages*EFI_PAGE_SIZE;
            break;
            case EfiBootServicesData:
            map = true;
            write = true;
            user = false;
            nx = true;
            mType = MMAP_TYPE_FREE;
            usable += descriptor->NumberOfPages*EFI_PAGE_SIZE;
            break;
            case EfiConventionalMemory:
            map = true;
            write = true;
            user = false;
            nx = false;
            mType = MMAP_TYPE_FREE;
            usable += descriptor->NumberOfPages*EFI_PAGE_SIZE;
            break;
            case EfiReservedMemoryType:
            map = false;
            mType = MMAP_TYPE_RESV;
            break;
            case EfiUnusableMemory:
            map = false;
            mType = MMAP_TYPE_UNUSABLE;
            break;
            case EfiRuntimeServicesCode:
            map = true;
            write = false;
            user = false;
            nx = false;
            mType = MMAP_TYPE_UEFI;
            break;
            case EfiRuntimeServicesData:
            map = true;
            write = true;
            user = false;
            nx = true;
            mType = MMAP_TYPE_UEFI;
            break;
            case EfiLoaderCode:
            map = true;
            write = false;
            user = false;
            nx = false;
            mType = MMAP_TYPE_FREE;
            usable += descriptor->NumberOfPages*EFI_PAGE_SIZE;
            break;
            case EfiLoaderData:
            map = true;
            write = true;
            user = false;
            nx = true;
            mType = MMAP_TYPE_FREE;
            usable += descriptor->NumberOfPages*EFI_PAGE_SIZE;
            break;
            case EfiACPIReclaimMemory:
            map = true;
            write = true;
            user = false;
            nx = true;
            mType = MMAP_TYPE_ACPI;
            usable += descriptor->NumberOfPages*EFI_PAGE_SIZE;
            break;
            case EfiACPIMemoryNVS:
            case EfiPalCode:
            map = true;
            write = true;
            user = false;
            nx = true;
            mType = MMAP_TYPE_ACPI;
            usable += descriptor->NumberOfPages*EFI_PAGE_SIZE;
            break;
        }
        if(map){
            mapPages(descriptor->PhysicalStart,descriptor->PhysicalStart,DESC_SZ(descriptor)/PAGE_SZ,write,user,nx);
        }
        if(entry[entryn-1].type == mType){
            --entryn;
            //Condense block to avoid fragmentation
            entry[entryn].bytes += DESC_SZ(descriptor);
            goto next;
        }
        entry[entryn].type = mType;
        entry[entryn].phys = descriptor->PhysicalStart;
        entry[entryn].virt = descriptor->PhysicalStart;
        entry[entryn].bytes = DESC_SZ(descriptor);
        next:
        descriptor = NEXT_DESC(descriptor,descSz);
    }
    kinf.mem.entries = entryn;
    kinf.mem.usableMem = usable;
    kinf.mem.totalMem = sz;
}