#include <stdint.h>
#include <string.h>
#include <debug.h>
#include "idt.h"

//Should be aligned on a dword boundary for better performance in the future
struct idtPointer idtp;
struct idtEntry idtEntries[256];

void writeIDT(struct idtEntry *entries, uint32 n, uint16 selector, void *isr){
    entries[n].type = 0xE;
    entries[n].selector = selector;
    entries[n].baseLower = IDT_BASE_LOWER((uint64)isr);
    entries[n].baseUpper = IDT_BASE_UPPER((uint64)isr);
    entries[n].present = true;
}

void overrideIDTEntry(uint32 vector, void *handle){
    writeIDT(&idtEntries,vector,0x8,handle);
}

void initIDT(){
    idtp.limit = (sizeof(struct idtEntry)*256)-1;
    idtp.base = (uint64)&idtEntries;
    kdebug(DINFO,"[IDTP|Limit: 0x%llx|Base: 0x%llx]\n",(uint64)idtp.limit,(uint64)idtp.base);
    memset(&idtEntries,0x0,sizeof(struct idtEntry)*256);
    writeIDT(&idtEntries,0,0x8,(void*)isr0);
    writeIDT(&idtEntries,1,0x8,(void*)isr1);
    writeIDT(&idtEntries,2,0x8,(void*)isr2);
    writeIDT(&idtEntries,3,0x8,(void*)isr3);
    writeIDT(&idtEntries,4,0x8,(void*)isr4);
    writeIDT(&idtEntries,5,0x8,(void*)isr5);
    writeIDT(&idtEntries,6,0x8,(void*)isr6);
    writeIDT(&idtEntries,7,0x8,(void*)isr7);
    writeIDT(&idtEntries,8,0x8,(void*)isr8);
    writeIDT(&idtEntries,9,0x8,(void*)isr9);
    writeIDT(&idtEntries,10,0x8,(void*)isr10);
    writeIDT(&idtEntries,11,0x8,(void*)isr11);
    writeIDT(&idtEntries,12,0x8,(void*)isr12);
    writeIDT(&idtEntries,13,0x8,(void*)isr13);
    writeIDT(&idtEntries,14,0x8,(void*)isr14);
    writeIDT(&idtEntries,15,0x8,(void*)isr15);
    writeIDT(&idtEntries,16,0x8,(void*)isr16);
    writeIDT(&idtEntries,17,0x8,(void*)isr17);
    writeIDT(&idtEntries,18,0x8,(void*)isr18);
    writeIDT(&idtEntries,19,0x8,(void*)isr19);
    writeIDT(&idtEntries,20,0x8,(void*)isr20);
    writeIDT(&idtEntries,21,0x8,(void*)isr21);
    writeIDT(&idtEntries,22,0x8,(void*)isr22);
    writeIDT(&idtEntries,23,0x8,(void*)isr23);
    writeIDT(&idtEntries,24,0x8,(void*)isr24);
    writeIDT(&idtEntries,25,0x8,(void*)isr25);
    writeIDT(&idtEntries,26,0x8,(void*)isr26);
    writeIDT(&idtEntries,27,0x8,(void*)isr27);
    writeIDT(&idtEntries,28,0x8,(void*)isr28);
    writeIDT(&idtEntries,29,0x8,(void*)isr29);
    writeIDT(&idtEntries,30,0x8,(void*)isr30);
    writeIDT(&idtEntries,31,0x8,(void*)isr31);
    writeIDT(&idtEntries,32,0x8,(void*)irq0);
    writeIDT(&idtEntries,33,0x8,(void*)irq1);
    writeIDT(&idtEntries,34,0x8,(void*)irq2);
    writeIDT(&idtEntries,35,0x8,(void*)irq3);
    writeIDT(&idtEntries,36,0x8,(void*)irq4);
    writeIDT(&idtEntries,37,0x8,(void*)irq5);
    writeIDT(&idtEntries,38,0x8,(void*)irq6);
    writeIDT(&idtEntries,39,0x8,(void*)irq7);
    writeIDT(&idtEntries,40,0x8,(void*)irq8);
    writeIDT(&idtEntries,41,0x8,(void*)irq9);
    writeIDT(&idtEntries,42,0x8,(void*)irq10);
    writeIDT(&idtEntries,43,0x8,(void*)irq11);
    writeIDT(&idtEntries,44,0x8,(void*)irq12);
    writeIDT(&idtEntries,45,0x8,(void*)irq13);
    writeIDT(&idtEntries,46,0x8,(void*)irq14);
    writeIDT(&idtEntries,47,0x8,(void*)irq15);
    kdebug(DINFO,"Set entries\n");
    loadIDT(&idtp);
    kdebug(DINFO,"Set IDT\n");
}