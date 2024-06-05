#include <stdint.h>
#include <ramdisk.h>
#include <elf.h>
#include <pmm.h>
#include <hashmap.h>
#include <kernel.h>

/*
For linking drivers to the kernel at runtime
*/

hashmap *ksym = NULL;

bool initKsym(){
    //Get symbol table
    struct rdFile *fl = NULL;//openRamdiskFile("fortuna.sym");
    if(fl == NULL){kpanic("No file for driver bindings",0);}
    ELF64Header *header = (ELF64Header*)fl;
    //Get symtab
    ELF64Section *sec = (ELF64Section*)((uint64_t)fl+header->shoff);
    void *symbolTable = NULL;
    if(symbolTable == NULL){kpanic("No symtab in elf",0);}
    //Parse symbol table for relevant bindings
    return false;    
}
