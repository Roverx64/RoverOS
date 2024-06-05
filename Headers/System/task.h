#pragma once
#include <stdint.h>
#include <vmm.h>

typedef uint64_t pid_t;

#define TASK_PRIORITY_LOW 0
#define TASK_PRIORITY_NORMAL 1
#define TASK_PRIORITY_HIGH 2
#define TASK_PRIORITY_SYSTEM 3
#define TASK_MAX_PRIORITY 3

#define TASK_TIMESLICE(p) ((p+1)*200)

//Task structure is loaded into the GS register
static inline uint64_t getCurrentTask(){
    uint64_t r = 0x0;
    asm volatile("movq %%gs, %0":"=r"(r):);
    return r;
}

struct taskTicket{
    pid_t id;
    uint8_t priority;
    pageSpace *space;
    char *name;
    uint8_t state;
    uint64_t rsp; //Task's stack
    uint64_t krsp; //Kernel stack
};

//Main ring 
struct taskQueue{
    uint8_t priority;
    uint32_t queueMaxSize;
    uint32_t enQueue;
    struct taskTicket **ticket;
    uint32_t ticketCursor;
};