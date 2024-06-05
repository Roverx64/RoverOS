#include <stdint.h>
#include <kernel.h>
#include <debug.h>
#include <cpu.h>

intHandler divByZero(registers reg){
    kpanic("Divide by zero",0);
    for(;;){asm("hlt");}
}