#include <efi.h>
#include <efilib.h>
#include "types.h"
#include "boot.h"
#include "relf.h"
#include "functions.h"

EFI_STATUS loadElf(EFIFile *elf){
    //Debug
    Print(L"Loading ELF file %s with size: 0x%llx\n",elf->name,elf->Info->FileSize);
    //Get space
    uintptr *buffer = getSection((uint64)elf->Info->FileSize,0x0);
    if((uint64)buffer == GS_INVALID_PTR){Print(L"Not enough space to load ELF\n"); return !EFI_SUCCESS;}
    //Read data to buffer
    uefi_call_wrapper(elf->File->Read,3,elf->File,&elf->Info->FileSize,buffer);
    //Copy header
    struct ELF64Header header;
    CopyMem(&header,buffer,sizeof(struct ELF64Header));
    //Check magic
    Print(L"ELF Magic: 0x%lx",ELF_READ32(header.ident.magic));
    /*This solution might be bad, but it works for now*/
    if(ELF_READ32(header.ident.magic) != ELF64_MAGIC){Print(L" = Invalid\n"); return !EFI_SUCCESS;}else{Print(L" = Valid\n");}
    if(header.ident.class != ELF_CLASS64){Print(L"Kernel not compiled for x64 (64 bit)\n"); return !EFI_SUCCESS;}
    return EFI_SUCCESS;
}
