#include <stdint.h>
#include <string.h>
#include "debug.h"
#include "paging.h"
#include "pmm.h"
#include "kmalloc.h"
#include "elf.h"

void loadProgHeader(pageSpace *procmap, ELF64ProgramHeader *p, void *elf){
}

bool checkMagic(ELF64_Byte *s, ELF64_Byte *magic){
    for(int i = 0;magic[i] != 'F'; ++i){
        if(s[i] != magic[i]){return false;}
    }
    return true;
}

uint64_t loadElf(pageSpace *procmap, void *elf){
    return 0x0;
}