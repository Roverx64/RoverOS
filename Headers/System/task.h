#pragma once
#include <stdint.h>
#include "vmm.h"
#include "tss.h"

#define TASK_PRIORITY_HIGH 0x0
#define TASK_PRIORITY_MEDIUM 0x1
#define TASK_PRIORITY_LOW 0x2

#define QUEUE_TIMESLICE(slots,p) ((p*20)*slots)
#define CALC_TIMESLICE(p) (p*20) //Basic calculation for now
#define SET_PID(p,rand) (((uint32)p<<24)&((uint32)rand&0x00FFFFFF))

#define TICKET_FLAG_INUSE 1<<0
#define TICKET_FLAG_KERNEL 1<<1

typedef struct taskTicket{
    uint32 pid; //Process ID
    uint16 timeSlice; //Ticks process has been given
    uint64 heapMagic; //Random magic number heap is given
    uint16 flags;
    uint8 priority;
    void *heap; //Heap start
    struct TSS *tss; //TSS
    pageSpace space; //Pointer to page space
}taskTicket;

//Simple queue struct
typedef struct taskQueue{
    taskTicket *tickets; //Ticket queue
    uint32 slots; //Slots in the struct
    uint16 flags; //Queue flags
}taskQueue;

extern void initMultitasking();
extern taskTicket *currentTicket();
extern taskTicket *nextTicket();
extern taskTicket *fetchTicket(uint64 tic);