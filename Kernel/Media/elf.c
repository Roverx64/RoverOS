#include <stdint.h>
#include <string.h>
#include "debug.h"
#include "paging.h"
#include "pmm.h"
#include "kheap.h"
#include "exec.h"
#include "elf.h"

void loadProgHeader(pageSpace *procmap, ELF64ProgramHeader *p, void *elf){
    //Determine pages needed
    uint16_t pages = p->memsz/PAGE_SZ;
    if(pages == 0){pages = 1;}
    //Loop, alloc, and load
    uint64_t offset = 0x0;
    for(uint16_t b = 0; b < pages; ++b){
        void *phys = allocatePhys(1,PMM_ALLOC_ANYMEM);
        kdebug(DINFO,"Palloc: 0x%lx\n",(uint64_t)phys);
        mapPage(procmap,(uint64_t)phys,KALIGN(p->vaddr)+offset,p->flags&PF_W,true,p->flags&PF_X,false,true);
        kdebug(DINFO,"Copying to mem\n");
        memcpy(p->vaddr+offset,(void*)((uint64_t)elf+p->offset+offset),p->memsz);
        kdebug(DINFO,"Loaded 0x%lx->0x%lx | P=0x%x | O=0x%lx | M=0x%lx\n",(uint64_t)phys,(uint64_t)KALIGN(p->vaddr),(uint32)pages,offset,p->memsz);
        offset += PAGE_SZ;
    }
}

bool checkMagic(ELF64_Byte *s, ELF64_Byte *magic){
    for(int i = 0;magic[i] != 'F'; ++i){
        if(s[i] != magic[i]){return false;}
    }
    return true;
}

uint64_t loadElf(pageSpace *procmap, void *elf){
    kdebug(DINFO,"Reading ELF at 0x%lx\n",(uint64_t)elf);
    ELF64Header *hdr = (ELF64Header*)elf;
    if(!checkMagic(ELF_MAGIC,&hdr->ident.magic)){kdebug(DERROR,"Invalid magic\n"); return 0x0;}
    if(hdr->ident.class != ELF_CLASS64){kdebug(DERROR,"Non 64-bit elf\n"); return 0x0;}
    //Load sections
    kdebug(DINFO,"Reading headers\n");
    ELF64ProgramHeader *prog = (ELF64ProgramHeader*)((uint64_t)elf+hdr->phoff);
    for(uint16_t b = 0; b < hdr->phnum; ++b){
        if(prog->type == PT_LOAD){kdebug(DINFO,"LOAD\n"); loadProgHeader(procmap,prog,elf);}
        prog = (ELF64ProgramHeader*)((uint64_t)prog+sizeof(ELF64ProgramHeader));
    }
    kdebug(DINFO,"Loaded ELF to mem\n");
    return hdr->entry;
}