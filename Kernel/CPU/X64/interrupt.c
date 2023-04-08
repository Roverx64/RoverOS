#include <stdint.h>
#include <string.h>
#include <debug.h>
#include <registers.h>
#include "interrupt.h"
#include "idt.h"

intHandler handlers[30];

void isr(registers reg, uint32 ec){
    if(handlers[ec] != NULL){
        intHandler fnc  = (intHandler*)handlers[ec];
        fnc(reg);
        return;
    }
    kdebug(DERROR,"Unhandled ISR\n");
    for(;;){asm("hlt");}
}

void irq(registers reg){
    kdebug(DINFO,"Got IRQ\n");
    for(;;){asm("hlt");}
}

void addInterruptHandler(uint32 vector, intHandler handler){
    handlers[vector] = handler;
}