#include <stdint.h>
#include <stdio.h>
#include "task.h"
#include "kheap.h"
#include "debug.h"
#include "pmm.h"
#include "paging.h"
#include "exec.h"
#include "gdt.h"
#include "tss.h"

taskQueue queues[4];
taskTicket *cticket;

bool addTicket(uint8 priority,uint16 flags,uintptr *tss,uintptr *heap){
    taskTicket *ticket = NULL;
    for(int i;i < queues[priority].slots;++i){
        if(queues[priority].tickets[i].flags&TICKET_FLAG_INUSE == 0x0){
            ticket = &queues[priority].tickets[i];
            goto end;
        }
    }
    //Realloc
    end:
    ticket->pid = SET_PID(priority,0x0);
    ticket->timeSlice = CALC_TIMESLICE(priority);
    ticket->heapMagic = 0x0; /*Randomize eventually*/
    ticket->flags = TICKET_FLAG_INUSE;
    ticket->priority = priority;
    ticket->heap = heap;
    ticket->tss = tss;
}

void initQueues(){
    //
}

taskTicket *createTicket(){
    taskTicket *ticket = (taskTicket*)halloc(sizeof(taskTicket));
}

void initMultitasking(){
    cticket = (taskTicket*)halloc(sizeof(taskTicket));
    cticket->pid = 0;
    cticket->timeSlice = CALC_TIMESLICE(TASK_PRIORITY_HIGH);
    cticket->heapMagic = 0x0;
    cticket->flags = 0x0;
    cticket->priority = TASK_PRIORITY_HIGH;
    cticket->heap = NULL;
    cticket->tss = NULL;
    uint64 pml;
    asm volatile("movq %%cr3, %0":"=r" (pml):);
    cticket->space.pml4e = (uint64*)pml;
}

pageSpace *getTaskPageSpace(uint32 pid, bool current){
    return &cticket->space;
}

//Basic roundrobin for testing
taskTicket tskq[10];
uint32 ctsk = 0;

taskTicket *currentTicket(){
    return &tskq[ctsk];
}

taskTicket *fetchTicket(uint64 tic){
    return &tskq[tic];
}

taskTicket *nextTicket(){
    if(ctsk+1 > 9){return &tskq[0];}
    return &tskq[ctsk+1];
}

void incTask(){
    if(ctsk+1 > 9){ctsk = 0; return;}
    ++ctsk;
}