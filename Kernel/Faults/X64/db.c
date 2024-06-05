#include <stdint.h>
#include <kernel.h>
#include <debug.h>
#include <cpu.h>

intHandler debugException(registers reg){
    kpanic("Debug exception",0);
    for(;;){asm("hlt");}
}