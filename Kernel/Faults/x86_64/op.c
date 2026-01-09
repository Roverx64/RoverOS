#include <stdint.h>
#include <kprint.h>
#include <fdebug.h>
#include <cpu.h>

void invalidOpcode(registers *reg){
    knone("#===========Invalid=Opcode===========#\n");
    dumpRegisters(reg);
    for(;;){asm("hlt");}
}