#include <stdint.h>
#include <kmalloc.h>
#include <mutex.h>
#include <task.h>
#include <vmm.h>
#include <lapic.h>
#include <string.h>
#include <kcache.h>
#include <bootinfo.h>
#include <kprint.h>
#include <kernel.h>
#include <sysinf.h>

//CPU specific structs
//GS is used as the index

struct localCPUInfo *cpuinf = NULL;
kcache *ticketCache = NULL;

//cpu.c
extern uint64_t gdtEntries[8];

mutex_t queueLock = 0x0;

kcacheConstructor *constructor(){

}

kcacheDestructor *destructor(){

}

struct cpuScheduler *scheduler;
static uint32_t inQueue = 1; //Tasks in the queue waiting for a time slice
uint32_t suspendedQueueSize = 0; //Max queue size
//Array of pointers to tickets currently blocked
struct processTicket **suspendedTickets;

/*!
    !D Creates a process ticket
    !I name: process name
    !I priority: process priority
    !I proc: process procmap
    !I stack: process stack virtual address
    !R Returns process ticket on success
    !R NULL on error
*/
struct processTicket *createTicket(char *name, uint8_t priority, vMarker *vmem, uint64_t stack){
    GET_LOCK(queueLock);
    struct processTicket *ticket = kcacheAlloc(ticketCache,0x0);
    memset(ticket,0x0,sizeof(struct processTicket));
    ticket->name = name;
    ticket->priority = priority;
    ticket->vmem = vmem;
    ticket->processID = 0;
    //ticket->tss = initTSS();
    return ticket;
}

/*!
    !D Fetches the current CPU's task
    !I None
    !R Returns the process ticket for the current task
*/
struct processTicket *getCurrentTask(){
    //Read GS for scheduler info
    uint32_t i = getPerCPUInfo();
    return scheduler[i].currentTask;
}

void nextTask(){

}

/*!
    !D Gets the current process' ID
    !I None
    !R Returns process ID
*/
pid_t getCurrentTaskID(){
    uint32_t i = getPerCPUInfo();
    return (uint64_t)scheduler[i].currentTask->processID;
}

/*!
    !D Suspends a process (IO/blocking)
    !I id: process id to suspend
    !R None
*/
void suspendProcess(uint64_t id){
    //Read GS for CPU's id
    uint32_t cpu = getPerCPUInfo();
    scheduler[cpu].currentTask->flags |= TASK_FLAG_BLOCKED;
    //switchTask(sche);
    return;
}

/*!
    !D Unsuspends a process
    !I id: process id to unsuspend
    !R None
*/
void unsuspendProcess(uint64_t id){
    for(uint32_t i = 0; i < suspendedQueueSize; ++i){
        if(suspendedTickets[i]->blocker != id){continue;}
        suspendedTickets[i]->flags &= ~TASK_FLAG_BLOCKED;
        return;
    }
    return;
}

/*!
    !D Switches a task on 'cpu' core
    !I cpu: CPU core to switch
    !R None
    !C NOTE: Do the task switch!
*/
void switchTask(uint32_t cpu){
    if(inQueue <= 1){
        //Idle and wait for an interrupt
        while(scheduler[cpu].currentTask->flags&TASK_FLAG_BLOCKED != 0){
            asm volatile("hlt");
        }
        return; //Process is unsuspended
    }
    //Switch task here
}

struct cpuScheduler fschedule;
struct processTicket ftask;

/*!
    !D Sets an error code for a process
    !I err: Error code to set
    !R None
    !C TODO: Copy to user accessible area later
*/
void setErrorCode(int err){
    struct processTicket *ticket = getCurrentTask();
    ticket->ec = err;
}

/*!
    !D Returns a process' error code
    !R error code
*/
int getErrorCode(){
    struct processTicket *ticket = getCurrentTask();
    return ticket->ec;
}

/*!
    !D Intilizes the multitasking structures
    !I kinf: Pointer to bootInfo passed from bootloader
    !R None
*/
void initTasking(struct bootInfo *kinf){
    kinfo("Intilizing task structures\n");
    uint32_t cores = 1;//sysinf.cpu->coreCount;
    KASSERT(cores,"No core count");
    //Ticket cache for faster allocations
    ticketCache = kcacheCreate("ticketCache",sizeof(struct processTicket),0,MEMORY_TYPE_ANY,0,constructor,destructor);
    KASSERT((uint64_t)ticketCache,"Creating kcache failed");
    kinfo("Created kcache\n");
    scheduler = (struct cpuScheduler*)kmalloc(sizeof(struct cpuScheduler)*cores);
    scheduler->currentTask = &ftask;
    kinfo("Allocated cpu structures with 0x%x cores\n",cores);
    //Create kernel ticket
    uint64_t stack = 0;
    asm volatile("movq %%rbp, %0":"=r"(stack):);
    struct processTicket *kt = createTicket((char*)"kernel",TASK_MAX_PRIORITY,ftask.vmem,stack);
    kt->space = &kspace;
    scheduler->currentTask = kt;
    //cpuinf[0]
    kinfo("Intilized multitasking\n");
}

vMarker *currentProcessVMarker(){
    struct processTicket *t = getCurrentTask();
    if(!t){return NULL;}
    return t->vmem;
}

/*!
    !D Creates a fake kernel task to use for bootstrapping memory
    !R None
*/
void createFakeTask(){
    kinfo("Creating fake task\n");
    setPerCPUInfo(0);
    swapPerCPUInfo();
    setPerCPUInfo(0);
    //Only these are used during bootstrap
    ftask.vmem = (vMarker*)kmalloc(sizeof(vMarker));
    ftask.vmem->base = 0x0;
    ftask.vmem->pages = 1;
    ftask.vmem->flags = 0;
    ftask.vmem->next = NULL;
    ftask.space = &kspace;
    fschedule.currentTask = &ftask;
    scheduler = &fschedule;
    kinfo("Created fake task\n");
}