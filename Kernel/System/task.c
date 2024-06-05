#include <stdint.h>
#include <kmalloc.h>
#include <mutex.h>
#include <task.h>
#include <vmm.h>
#include <lapic.h>
#include <string.h>
#include <kcache.h>

struct taskQueue queue[TASK_MAX_PRIORITY+1];
uint8_t queueCursor = TASK_MAX_PRIORITY; //Downwards counting
mutex_t taskLock = 0;
extern void switchTask(uint64_t rsp, uint64_t cr3);
kcache *ticketCache;

bool addTicket(struct taskTicket *ticket){
    GET_LOCK(taskLock);
    if(ticket == NULL){FREE_LOCK(taskLock); return false;}
    if(ticket->priority > TASK_MAX_PRIORITY){FREE_LOCK(taskLock); return false;}
    uint8_t p = ticket->priority;
    if(queue[p].enQueue == queue[p].queueMaxSize){
        queue[p].ticket = (struct taskTicket*)krealloc(queue[p].ticket,sizeof(struct taskTicket)*queue[p].queueMaxSize+10);
        memset((void*)((uint64_t)queue[p].ticket+queue[p].queueMaxSize),0x0,sizeof(struct taskTicket)*10);
        queue[p].queueMaxSize += 10;
    }
    //Find free ticket
    for(uint32_t i = 0; i < queue[p].queueMaxSize; ++i){
        if(queue[p].ticket[i] != NULL){continue;}
        queue[p].enQueue += 1;
        memcpy(&queue[p].ticket[i],ticket,sizeof(struct taskTicket));
        break;
    }
    FREE_LOCK(taskLock);
    return true;
}

void nextTask(){
    //Get APIC id to identify CPU
    uint16_t id = getLapicID();
    GET_LOCK(taskLock);
    struct taskTicket *tsk = &queue[queueCursor].ticket[queue[queueCursor].ticketCursor];
    if(queue[queueCursor].ticketCursor == 0){
        queue[queueCursor].ticketCursor = queue[queueCursor].enQueue;
    }
    else{
        queue[queueCursor].ticketCursor -= 1;
    }
    if(queueCursor == 0){queueCursor = TASK_MAX_PRIORITY;}
    else{--queueCursor;}
    FREE_LOCK(taskLock);
    //Set RSP0
    //cpu[id].tss->RSP0 = tsk->krsp;
    //Set lapic
    //writeNewTaskTimer(id, TASK_TIMESLICE(tsk->priority));
    //Switch task
    switchTask(tsk->rsp,(uint64_t)tsk->space->pml4e);
}

extern void contextSwitch(void);

//Setups kernel's task and ticket
void initTasking(){
    struct taskTicket kticket;
    kticket.id = 0;
    kticket.priority = TASK_PRIORITY_SYSTEM;
    kticket.rsp = 0x0;
    kticket.krsp = 0x0;
    //Intilize task structures
    for(int i = 0; i < TASK_MAX_PRIORITY; ++i){
        queue[i].priority = i;
        queue[i].queueMaxSize = 30;
        queue[i].enQueue = 0;
        queue[i].ticket = kmalloc(sizeof(struct taskTicket)*30);
        memset(queue[i].ticket,0x0,sizeof(struct taskTicket)*30);
    }
    //Add kernel ticket
    addTicket(&kticket);
    //Override IDT entry for preemptive task switching
    overrideIDTEntry(255,contextSwitch);
}
