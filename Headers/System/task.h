#pragma once
#include <stdint.h>
#include <vmm.h>
#include <cpu.h>

typedef uint64_t pid_t;

#define TASK_PRIORITY_LOW 0
#define TASK_PRIORITY_NORMAL 1
#define TASK_PRIORITY_HIGH 2
#define TASK_PRIORITY_SYSTEM 3
#define TASK_MAX_PRIORITY 3

#define TASK_TIMESLICE(p) ((p+1)*500)

#define TASK_FLAG_BLOCKED 0x1

struct processTicket{
    char *name;
    uint8_t priority;
    uint16_t flags;
    pid_t processID;
    uint64_t blocker; //ID set by caller
    uint32_t gs; //Shared by ther kernel and process
    uint32_t fs;
    int ec; //Last error code recieved
    vMarker *vmem;
    struct TSS *tss;
    pageSpace *space;
};

struct taskQueue{
    struct processTicket **tickets;
    uint32_t ticketCursor;
    uint32_t maxTickets;
    uint32_t ticketCount; 
};

//Per-CPU
//Keeps track of scheduling information
struct cpuScheduler{
    struct localCPUInfo cpu;
    struct processTicket *currentTask;
    uint32_t taskCursor;
};

extern struct processTicket *getCurrentTask();

extern void suspendProcess(uint64_t id);
extern void unsuspendProcess(uint64_t id);
extern uint64_t getCurrentTaskID();
extern vMarker *currentProcessVMarker();