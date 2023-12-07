#include <stdint.h>
#include "testmod.h"
#include "mod.h"

struct testMod table;

//Called by mod loader
modent *getTable(){
    table.id = (uint64)0xFEEFFABFC;
    return (void*)&table;
}

void modInit(){
    asm volatile("movq $0xFEEFFABFC, %%r12":::"rdi");
    asm volatile("movq $0x1, %%rdi;\n\t""syscall":::"rdi");
    asm volatile("movq $0x1, %%rdi;\n\t""syscall":::"rdi");
    for(;;){}
}