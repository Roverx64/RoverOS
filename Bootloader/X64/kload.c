#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "bootFile.h"
#include "bootPaging.h"
#include "elf.h"

EFIFile kbin;

bool checkMagic(ELF64_Byte *s, ELF64_Byte *magic){
    for(int i = 0;magic[i] != 'F'; ++i){
        if(s[i] != magic[i]){return false;}
    }
    return true;
}

uint64 bootstrapKernel(EFI_HANDLE ImageHandle){
    ZeroMem(&kbin,sizeof(EFIFile));
    if(openVolume(&kbin,ImageHandle) != EFI_SUCCESS){Print(L"KLOAD : openVolume() failed\n"); return 0x0;}
    if(openFile(&kbin,L"Fortuna.bin",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY) != EFI_SUCCESS){Print(L"KLOAD : Failed to open Fortuna.bin\n"); return 0x0;}
    if(getFileInfo(&kbin) != EFI_SUCCESS){Print(L"KLOAD : getFileInfo() failed\n"); return 0x0;}
    Print(L"KLOAD : Successfully opened Fortuna.bin\n");
    //Check magic
    ELF64Header header;
    readFile(&kbin,&header,sizeof(ELF64Header));
    if(checkMagic((ELF64_Byte*)ELF_MAGIC,(ELF64_Byte*)header.ident.magic) != true){Print(L"KLOAD : Invalid ELF magic\n"); return 0x0;}
    Print(L"KLOAD : Validated ELF magic\n");
    //Check for 64-Bit compat
    if(header.ident.class != ELF_CLASS64){Print(L"KLOAD : ELF is not 64-Bit\n"); return 0x0;}
    Print(L"KLOAD : ELF is 64 bit\n");
    //Determine kernel size
    bool write = false;
    bool execute = false;
    ELF64ProgramHeader prog;
    uint64 kernelSize = 0x0;
    uint64 kernelBase = 0x0;
    uint64 kernelOffset = 0x0;
    uint64 sections = 0x0;
    if(setFilePos(&kbin,header.phoff) != EFI_SUCCESS){Print(L"KLOAD : setFilePos() failed\n"); return 0x0;}
    for(uint32 i = 0; i < header.phnum; ++i){
        if(readFile(&kbin,&prog,sizeof(ELF64ProgramHeader)) != EFI_SUCCESS){Print(L"KLOAD : readFile() failed\n"); return 0x0;}
        if(prog.type != PT_LOAD){continue;}
        kernelSize += prog.memsz;
        ++sections;
    }
    kernelSize += sections*PAGE_SZ; //For alignment
    kernelBase = (uint64)AllocatePool(kernelSize+PAGE_SZ);
    ZeroMem((void*)kernelBase,kernelSize);
    if(kernelBase == 0x0){Print(L"KLOAD : AllocatePool() failed\n"); return 0x0;}
    kernelBase = (uint64)KALIGN(kernelBase)+PAGE_SZ;
    //Set bootinfo
    kinf.load.phys = kernelBase;
    kinf.load.size = kernelSize;
    //Load sections and map to higher half
    for(uint32 i = 0; i < header.phnum; ++i){
        if(setFilePos(&kbin,header.phoff+(sizeof(ELF64ProgramHeader)*i)) != EFI_SUCCESS){Print(L"KLOAD : setFilePos() failed\n"); return 0x0;}
        if(readFile(&kbin,&prog,sizeof(ELF64ProgramHeader)) != EFI_SUCCESS){Print(L"KLOAD : readFile() failed\n"); return 0x0;}
        if(prog.type != PT_LOAD){continue;}
        Print(L"KLOAD : [0x%lx->0x%lx|",kernelBase+kernelOffset,prog.vaddr);
        bool write = false;
        bool exec = false;
        if(prog.flags&PF_R){Print(L"R");}else{Print(L"-");}
        if(prog.flags&PF_W){Print(L"W"); write = true;}else{Print(L"-");}
        if(prog.flags&PF_X){Print(L"X"); exec = true;}else{Print(L"-");}
        Print(L"|0x%lx|0x%lx]\n",prog.memsz,(prog.memsz/PAGE_SZ));
        if(setFilePos(&kbin,prog.offset) != EFI_SUCCESS){Print(L"KLOAD : setFilePos() failed\n"); return 0x0;}
        if(readFile(&kbin,(void*)(kernelBase+kernelOffset),prog.filesz) != EFI_SUCCESS){Print(L"KLOAD : readFile() failed\n"); return 0x0;}
        mapPages((kernelBase+kernelOffset),prog.vaddr,(prog.memsz/PAGE_SZ)+1,write,false,exec);
        kernelOffset += KALIGN(prog.memsz+PAGE_SZ);
        if((prog.vaddr+prog.memsz) > kinf.load.endOfKernel){kinf.load.endOfKernel = prog.vaddr+prog.memsz;}
    }
    //Cleanup
    closeFile(&kbin);
    closeVolume(&kbin);
    Print(L"KLOAD : Loaded kernel with entry at 0x%lx\n",header.entry);
    return header.entry;
}

EFI_STATUS setupKheap(){
    kinf.load.kheapSize = 0x200000*4;
    kinf.load.kheapBase = (uint64)AllocatePool(kinf.load.kheapSize);
    if(kinf.load.kheapBase == 0x0){Print(L"HEAP  : AllocatePool() failed\n"); return !EFI_SUCCESS;}
    kinf.load.kheapVBase = kinf.load.kheapBase;
    mapPages(kinf.load.kheapBase,kinf.load.kheapBase,kinf.load.kheapSize/PAGE_SZ,true,false,false);
    Print(L"HEAP  : Allocated %xMB heap to 0x%lx\n",kinf.load.kheapSize/0x200000,kinf.load.kheapBase);
    kinf.load.slabSize = 0x200000*4;
    //TODO: Remove slab of memory in favor of dynamic and non contiguious memory
    //Using virtToPhys function when allocating new tables.
    kinf.load.pageSlab = AllocatePool(kinf.load.slabSize);
    if(kinf.load.pageSlab == 0x0){Print(L"HEAP  : AllocatePool() failed\n"); return !EFI_SUCCESS;}
    kinf.load.pageSlab = KALIGN(kinf.load.pageSlab+0x1000);
    mapPages(kinf.load.pageSlab,kinf.load.pageSlab,kinf.load.slabSize/PAGE_SZ,true,false,false);
    Print(L"HEAP  : Allocated %xMB slab to 0x%lx\n",kinf.load.slabSize/0x200000,kinf.load.pageSlab);
    return EFI_SUCCESS;
}

EFI_STATUS loadRamdisk(){

}