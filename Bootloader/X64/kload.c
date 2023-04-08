#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "boottypes.h"
#include "bootMmap.h"
#include "bootinfo.h"
#include "bootFile.h"
#include "bootPaging.h"
#include "elf.h"

void *kernelBase = NULL;
void *ramdiskBase = NULL;
EFIFile kfile;
uint64 kentry = 0x0;

void loadToMem(ELF64ProgramHeader *prog){
    Print(L"[");
    bool write;
    bool x;
    if(prog->flags&PF_R){Print(L"R");}else{Print(L"-");}
    if(prog->flags&PF_W){Print(L"W"); write = true;}else{Print(L"-"); write = false;}
    if(prog->flags&PF_X){Print(L"X"); x = true;}else{Print(L"-"); x = false;}
    Print(L"|Offset: 0x%llx|0x%llx->0x%llx",prog->offset,KALIGN((uint64)kernelBase+prog->offset),prog->vaddr);
    Print(L"|Sz: 0x%llx|0x%llx pages|Align: 0x%llx]\n",prog->memsz,prog->memsz/PAGE_SZ,prog->align);
    setFilePos(&kfile,prog->offset);
    readFile(&kfile,(void*)((uint64)kernelBase+prog->offset),prog->memsz);
    mapPages((uint64)kernelBase+prog->offset,prog->vaddr,prog->memsz/PAGE_SZ,write,false,!x);
}

EFI_STATUS loadSections(ELF64Header *header){
    setFilePos(&kfile,header->phoff);
    ELF64ProgramHeader program;
    for(int i = 0; i < header->phnum; ++i){
        readFile(&kfile,&program,sizeof(program));
        switch(program.type){
            case PT_LOAD:
                loadToMem(&program);
                setFilePos(&kfile,header->phoff+(sizeof(ELF64ProgramHeader)*(i+1)));
            break;
            default:
            break;
        }
    }
}

uint64 querySize(ELF64Header *header){
    ELF64ProgramHeader program;
    uint64 sz = 0x0;
    for(int i = 0; i < header->phnum; ++i){
        readFile(&kfile,&program,sizeof(program));
        if(program.type == PT_LOAD){sz += program.offset;}
        setFilePos(&kfile,header->phoff+(sizeof(ELF64ProgramHeader)*(i+1)));
    }
    return sz;
}

bool checkMagic(ELF64_Byte *s, ELF64_Byte *magic){
    for(int i = 0;magic[i] != 'F'; ++i){
        if(s[i] != magic[i]){return false;}
    }
    return true;
}

//Returns a pointer to the kernel entry
uint64 bootstrapKernel(EFI_HANDLE ImageHandle){
    //Setup and prepare kernel
    EFI_STATUS status = EFI_SUCCESS;
    status = openVolume(&kfile,ImageHandle);
    if(EFI_ERROR(status)){Print(L"Failed to open volume\n"); return 0x0;}
    status = openFile(&kfile,L"Fortuna.bin",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
    if(EFI_ERROR(status)){Print(L"Failed to open kernel file\n"); return status;}
    status = getFileInfo(&kfile);
    if(EFI_ERROR(status)){Print(L"Failed to get file info\n"); return status;}
    Print(L"Opened %s\n",kfile.name);
    //Read ELF
    ELF64Header header;
    readFile(&kfile,&header,sizeof(header));
    if(!checkMagic((ELF64_Byte*)ELF_MAGIC,(ELF64_Byte*)header.ident.magic)){Print(L"Invalid ELF magic"); return 0x0;}
    Print(L"Found valid ELF magic\n");
    uint64 ksz = querySize(&header)+PAGE_SZ;
    kernelBase = getMemory(ksz);
    if(kernelBase == NULL){Print(L"Failed to find space for the kernel\n"); return 0x0;}
    kernelBase = (void*)(KALIGN(kernelBase)+PAGE_SZ);
    kinf.load.phys = (uint64)kernelBase;
    kinf.load.virt = KALIGN(header.entry);
    Print(L"Loading kernel to phys 0x%llx\n",(uint64)kernelBase);
    loadSections(&header);
    kentry = (uint64)header.entry;
    status = closeFile(&kfile);
    if(EFI_ERROR(status)){Print(L"Failed to close file"); return 0x0;}
    //Load ramdisk
    status = openFile(&kfile,L"initrd.disk",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY);
    if(EFI_ERROR(status)){Print(L"Failed to open ramdisk\n"); return 0x0;}
    status = getFileInfo(&kfile);
    if(EFI_ERROR(status)){Print(L"Failed to get file info\n"); return 0x0;}
    ramdiskBase = getMemory(kfile.Info->FileSize);
    kinf.load.rdptr = (uint64)ramdiskBase;
    if(ramdiskBase == NULL){Print(L"Failed to get memory for ramdisk\n"); return 0x0;}
    readFile(&kfile,ramdiskBase,kfile.Info->FileSize);
    mapPages((uint64)ramdiskBase,(uint64)ramdiskBase,kfile.Info->FileSize/PAGE_SZ,true,false,true);
    closeFile(&kfile);
    Print(L"Loaded ramdisk to 0x%llx\n",kinf.load.rdptr);
    return (uint64)header.entry;
}