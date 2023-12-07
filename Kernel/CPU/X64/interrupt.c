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
    kdebug(DERROR,"Unhandled ISR 0x%x\n",ec);
    for(;;){asm("hlt");}
}

void irq(registers reg, uint64 irqn){
    if(irqn < 32){kdebug(DERROR,"Invalid IRQ of 0x%x\n",irqn); return;}
    if(handlers[irqn] == NULL){kdebug(DWARN,"Unhandled IRQ\n"); return;}
    intHandler fnc = (intHandler*)handlers[irqn];
    fnc(reg);
    return;
}

void addInterruptHandler(uint32 vector, intHandler handler){
    handlers[vector] = handler;
}