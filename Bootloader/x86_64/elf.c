#include <sefi.h>
#include <stdint.h>
#include <stdbool.h>
#include "bootFile.h"
#include "bootMem.h"
#include "elf.h"

bool checkMagic(ELF64_Byte *s, ELF64_Byte *magic){
    for(int i = 0;magic[i] != 'F'; ++i){
        if(s[i] != magic[i]){return false;}
    }
    return true;
}

uint64_t getELFMemSize(EFIFile *elf){
    if(setFilePos(elf,0) != EFI_SUCCESS){sprint(L"setFilePos() failed\n"); return 0x0;}
    ELF64Header header;
    readFile(elf,&header,sizeof(ELF64Header));
    if(setFilePos(elf,header.phoff) != EFI_SUCCESS){sprint(L"setFilePos() failed\n"); return 0x0;}
    ELF64ProgramHeader prog;
    uint64_t sz = 0;
    for(ELF64_Half i = 0; i < header.phnum; ++i){
        readFile(elf,&prog,sizeof(prog));
        if(prog.type != PT_LOAD){continue;}
        sz += prog.memsz;
    }
    return sz;
}

uint64_t loadELF(EFIFile *elf, bool kernel){
    if(setFilePos(elf,0) != EFI_SUCCESS){sprint(L"setFilePos() failed\n"); return 0x0;}
    ELF64Header header;
    readFile(elf,&header,sizeof(ELF64Header));
    if(checkMagic((ELF64_Byte*)ELF_MAGIC,(ELF64_Byte*)header.ident.magic) != true){sprint(L"Invalid magic\n"); return 0x0;}
    //32-Bit is not supported
    if(header.ident.class != ELF_CLASS64){sprint(L"Non 64-Bit binary\n"); return 0x0;}
    if(setFilePos(elf,header.phoff) != EFI_SUCCESS){sprint(L"setFilePos() failed\n"); return 0x0;}
    ELF64ProgramHeader prog;
    uint8_t type = (kernel == true) ? MMAP_TYPE_KERNEL : MMAP_TYPE_ELF; 
    //Look for LOAD sections
    for(ELF64_Half i = 0; i < header.phnum; ++i){
        readFile(elf,&prog,sizeof(prog));
        if(prog.type != PT_LOAD){continue;}
        sprint(L"Found LOAD section with ");
        if(prog.flags&PF_W){sprint(L"RW");}else{sprint(L"R-");}
        if(prog.flags&PF_X){sprint(L"X");}else{sprint(L"-");}
        sprint(L" flags\n");
        //Allocate memory
        uint64_t load = (uint64_t)allocMem(prog.memsz,prog.vaddr,type,false,true,true);
        if(prog.filesz == 0){goto map;}
        //Read file into memory
        uint64_t save = getFilePos(elf);
        setFilePos(elf,prog.offset);
        readFile(elf,(void*)load,prog.filesz);
        //Restore file position and map address
        setFilePos(elf,save);
        map:
        mapPages(load,prog.vaddr,(prog.memsz/PAGE_SZ)+1,prog.flags&PF_W,prog.flags&PF_X);
        sprint(L"Mapped section 0x%lx->0x%lx | 0x%lx pages\n",(uint64_t)load,prog.vaddr,(prog.memsz/PAGE_SZ)+1);
    }
    return (uint64_t)header.entry;
}

//0x...8000e430