#include <stdint.h>
#include <stdio.h>
#include <debug.h>
#include "task.h"
#include "gdt.h"
#include "tss.h"
#include "exec.h"
#include "kheap.h"
#include "bootinfo.h"
#include "pmm.h"

extern void modSwitch(uint64_t rip, uint64_t rsp, uint64_t rbp);
extern struct bootInfo *boot;
extern void testSwitch(uint64_t cr3);
extern pageSpace kspace;

bool loadModule(void *elf){
    //Initilize ticket
    taskTicket *ticket = createTicket();
    //Setup page space
    ticket->space.pml4e = (uint64_t*)halloc(sizeof(uint64_t)*512);
    memset(ticket->space.pml4e,0x0,sizeof(uint64_t)*512);
    //Setup TSS
    ticket->tss = newTSS();
    //Load ELF

    //Switch task
    return true;
}