#include <stdint.h>
#include <kernel.h>
#include <kprint.h>
#include <fdebug.h>
#include <cpu.h>

void divByZero(registers *reg){
    knone("#===========Divide=By=Zero===========#\n");
    dumpRegisters(reg);
    for(;;){asm("hlt");}
}