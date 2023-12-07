#include <stdint.h>
#include <string.h>
#include <debug.h>
#include "gdt.h"
#include "tss.h"
#include "task.h"
#include "kheap.h"
#include "paging.h"
#include "pmm.h"

struct TSS ktss;
extern uint64 gdtEntries[8];
extern void enableSyscall(void);
extern void setSyscallEntry(uint64);
extern void testSyscall(void);
extern void syscallEntryPrologue();

void testEntry(uint64 n){
    kdebug(DINFO,"N: 0x%lx\n",n);
    for(;;){asm("hlt");}
}

void initTSS(uint64 kstack){
    kdebug(DINFO,"Initilizing kernel TSS\n");
    kdebug(DINFO,"TSS at 0x%lx\n",&ktss);
    memset(&ktss,0x0,sizeof(ktss));
    ktss.RSP0 = kstack;
    ktss.iopb = sizeof(ktss);
    //Write TSS
    kdebug(DINFO,"Writing to GDT\n");
    writeGDTTSS(&gdtEntries,5,(uint64)&ktss,0);
    enableSyscall();
    kdebug(DINFO,"Enabled syscalls\n");
    //Load TSS
    loadTSS(0x28);
    kdebug(DINFO,"Loaded TR register\n");
    //Set Syscall entry
    setSyscallEntry(syscallEntryPrologue);
    kdebug(DINFO,"Wrote syscall entry to 0x%lx\n",syscallEntryPrologue);
}

struct TSS *newTSS(pageSpace *proc){
    struct TSS *tss = halloc(sizeof(struct TSS));
    if(tss == NULL){return NULL;}
    memset(tss,0x0,sizeof(struct TSS));
    tss->RSP0 = allocatePhys(1,PMM_ALLOC_ANYMEM); //Kernel stack
    tss->RSP2 = allocatePhys(1,PMM_ALLOC_ANYMEM); //Process stack
    tss->IST0 = allocatePhys(1,PMM_ALLOC_ANYMEM); //Interrupt stack
    //Temporary mappings
    //TODO: Properly map these into user/kernel CR3
    mapPage(proc,tss->RSP0,PAGE_SZ*1,true,false,false,false,true);
    mapPage(proc,tss->RSP2,PAGE_SZ*3,true,true,false,false,true);
    mapPage(proc,tss->IST0,PAGE_SZ*5,true,false,false,false,true);
    tss->RSP0 = PAGE_SZ*1;
    tss->RSP2 = PAGE_SZ*3;
    tss->IST0 = PAGE_SZ*5;
    return tss;
}