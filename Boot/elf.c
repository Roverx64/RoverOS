#include <efi.h>
#include <efilib.h>
#include "types.h"
#include "boot.h"
#include "relf.h"
#include "functions.h"

EFI_STATUS loadElf(EFIFile *elf){
    //Allocate space
    uintptr *buffer = (uintptr*)AllocatePool(elf->Info->FileSize);
    //Read data to buffer
    uefi_call_wrapper(elf->File->Read,3,elf->File,&elf->Info->FileSize,buffer);
    //Copy header
    struct ELF64Header header;
    CopyMem(&header,buffer,sizeof(struct ELF64Header));
    //Check magic
    Print(L"ELF Magic: 0x%llx",(uint64)header.ident.magic);
    /*Bytes are currently backwards in memory. I will have to fix this later*/
    if(header.ident.magic != ELF64_MAGIC){Print(L" = Invalid\n"); return !EFI_SUCCESS;}else{Print(L" = Valid\n");}
    return EFI_SUCCESS;
}
