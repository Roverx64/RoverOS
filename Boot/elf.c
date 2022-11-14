#include <efi.h>
#include <efilib.h>
#include "types.h"
#include "boot.h"
#include "relf.h"
#include "functions.h"

uintptr *ldbuffer = NULL;
uintptr *buffer = NULL;
ELF64Header *header;

void initSection(ELF64Section *section){
    switch(section->type){
        case SHT_NULL:
        //Ignore
        break;
        case SHT_PROGBITS:
        //Load to memory if Alloc flag is set
        if(section->flags&SHF_ALLOC){
            Print(L"[Progbits");
            CopyMem((uint8*)VTOP(ldbuffer,section->addr,header->entry),(uint8*)((uintptr)buffer+(uintptr)section->offset),section->size);
            goto extra;
        }
        break;
        case SHT_SYMTAB:
        break;
        case SHT_STRTAB:
        break;
        case SHT_RELA:
        break;
        case SHT_HASH:
        break;
        case SHT_DYNAMIC:
        break;
        case SHT_NOTE:
        break;
        case SHT_NOBITS:
        Print(L"[Nobits");
        //Clear memory
        SetMem((uint8*)VTOP(ldbuffer,section->addr,header->entry),section->size,0x0);
        goto extra;
        break;
        case SHT_REL:
        break;
        case SHT_SHLIB:
        break;
        case SHT_DYNSYM:
        break;
        default:
        Print(L"[Unknown|0x%lx]\n",section->type);
        goto end;
        break;
    }
    goto end;
    extra:
    Print(L"|Addr:0x%llx|Size:0x%llx|",section->addr,section->size);
    //I'll optimize this later
    if(section->flags&SHF_WRITE){Print(L"W");}else{Print(L"-");}
    if(section->flags&SHF_ALLOC){Print(L"A");}else{Print(L"-");}
    if(section->flags&SHF_EXEC){Print(L"X");}else{Print(L"-");}
    Print(L"]\n");
    //Check and set page mapping here eventually
    end:
    return;
}

void initProgramHeader(ELF64ProgramHeader *header){
    //We can ignore phys addr as we will be using only the virt addr anyways
    switch(header->type){
        case PT_NULL:
        break;
        case PT_LOAD:
        Print(L"[Load");
        goto extra;
        break;
        case PT_DYNAMIC:
        break;
        case PT_INTERP:
        break;
        case PT_NOTE:
        break;
        case PT_SHLIB:
        break;
        case PT_PHDR:
        break;
        Print(L"[Unknown|0x%lx]\n",header->type);
        default:
        break;
    }
    goto end;
    extra:
    Print(L"|Addr:0x%llx|Size:0x%llx|",header->vaddr,header->memsz);
    if(header->flags&PF_R){Print(L"R");}else{Print(L"-");}
    if(header->flags&PF_W){Print(L"W");}else{Print(L"-");}
    if(header->flags&PF_X){Print(L"X");}else{Print(L"-");}
    Print(L"]\n");
    end:
    return;
}

bool checkMagic(ELF64_Byte *s, ELF64_Byte *magic){
    for(int i = 0;magic[i] != 'F'; ++i){
        if(s[i] != magic[i]){return false;}
    }
    return true;
}

EFI_STATUS loadElf(EFIFile *elf){
    //Debug
    Print(L"Loading ELF file %s with size: 0x%llx\n",elf->name,elf->Info->FileSize);
    //Get space
    ldbuffer = getSection((uint64)elf->Info->FileSize,0x0);
    buffer = (uintptr*)AllocatePool((UINTN)elf->Info->FileSize);
    if(ldbuffer == buffer){Print(L"Got pointer to same phys addr\n"); return !EFI_SUCCESS;}
    if(((uint64)ldbuffer == GS_INVALID_PTR) || (buffer == NULL)){Print(L"Not enough space to load ELF\n"); return !EFI_SUCCESS;}
    //Read data to buffer
    uefi_call_wrapper(elf->File->Read,3,elf->File,&elf->Info->FileSize,buffer);
    header = (ELF64Header*)buffer;
    Print(L"%c%c%c|",header->ident.magic[1],header->ident.magic[2],header->ident.magic[3]);
    if(checkMagic((ELF64_Byte*)&header->ident.magic,ELF_MAGIC) == true){Print(L"Valid|");}else{Print(L"Invalid\n"); return !EFI_SUCCESS;}
    if(header->ident.class == ELF_CLASS64){Print(L"x64|");}else{Print(L"x86|Unsupported\n"); return !EFI_SUCCESS;}
    Print(L"Phys: 0x%llx|Virt: 0x%llx\n",ldbuffer,header->entry);
    //Read sections
    ELF64Section *entry = (ELF64Section*)((uintptr)buffer+header->shoff);
    for(int i = header->shnum; i > 0; --i){
        initSection(entry);
        entry = (ELF64Section*)((uintptr)entry+header->shentsize);
    }
    ELF64ProgramHeader *pentry = (ELF64ProgramHeader*)((uintptr)buffer+header->phoff);
    for(int i = header->phnum; i > 0; --i){
        initProgramHeader(pentry);
        pentry = (ELF64ProgramHeader*)((uintptr)pentry+header->phentsize);
    }
    return EFI_SUCCESS;
}