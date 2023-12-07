#include <stdint.h>
#include <stdio.h>
#include "task.h"
#include "kheap.h"
#include "paging.h"
#include "debug.h"

//TODO: Merge with task.c/module.c

void setupTask(taskTicket *ticket){
    //TSS
    //ticket->tss = (struct TSS*)halloca(sizeof(struct TSS));
    memset(ticket->tss,0x0,sizeof(struct TSS));
    kdebug(DNONE,"Allocated TSS at 0x%lx\n",(uint64)ticket->tss);
    //CR3
    //ticket->space.pml4e = (uint64*)halloca(sizeof(uint64)*512);
    memset(ticket->space.pml4e,0x0,sizeof(uint64)*512);
    kdebug(DNONE,"CR3 at 0x%lx\n",(uint64)ticket->space.pml4e);
    //Istack
    //ticket->tss->IST0 = (uint64)halloca(sizeof(uint64)*100);
    memset((uint64*)ticket->tss->IST0,0x0,sizeof(uint64)*100);
    kdebug(DNONE,"IST at 0x%lx\n",(uint64)ticket->tss->IST0);
    //Ustack
    //ticket->tss->RSP2 = (uint64)halloca(sizeof(uint64)*100);
    memset((uint64*)ticket->tss->RSP2,0x0,sizeof(uint64)*100);
    kdebug(DNONE,"User stack at 0x%lx\n",ticket->tss->RSP2);
    //Kstack
    //ticket->tss->RSP0 = (uint64)halloca(sizeof(uint64)*100);
    memset((uint64*)ticket->tss->RSP0,0x0,sizeof(uint64)*100);
    kdebug(DNONE,"Kernel stack at 0x%lx\n",ticket->tss->RSP0);
    //Disable IOPB
    ticket->tss->iopb = sizeof(struct TSS);
}

void switchTask(){
    taskTicket *new = nextTicket();
    taskTicket *current = currentTicket();
    //loadTR(new->tss);
    //incTask();
}