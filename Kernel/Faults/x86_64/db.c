#include <stdint.h>
#include <kernel.h>
#include <kprint.h>
#include <fdebug.h>
#include <cpu.h>

void debugException(registers *reg){
    knone("#==========Debug==Exception==========#");
    dumpRegisters(reg);
    for(;;){asm("hlt");}
}