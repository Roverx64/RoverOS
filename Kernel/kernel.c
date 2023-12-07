#include <stdint.h>
#include <gui.h>
#include "bootinfo.h"
#include <debug.h>
#include <kernel.h>
#include "ramdisk.h"
#include "pmm.h"
#include "idt.h"
#include "gdt.h"
#include "paging.h"
#include "kheap.h"
#include "task.h"
#include "syscall.h"

UIwindow kwindow;
extern void loadStack();
extern void initFaultHandlers();
extern void initACPI(struct bootInfo *kinf);
extern void initUIDebug(void *buffer, uint32 Xr, uint32 Yr);
extern void updateTextTest();
extern void testRead(void *psf);
extern uint64 lapicTick;
struct bootInfo *boot;
extern uint64 stackTop;
sysinfo sysinf;

syscallHandler testsys(uint64 in){
    kdebug(DNONE,"INPUT: 0x%lx\n",in);
}

void kmain(struct bootInfo *kinf){
    if(kinf->magic != BOOTINFO_MAGIC){
        kdebug(DERROR,"Invalid bootinfo magic\n");
        goto end;
    }
    boot = kinf;
    kdebug(DINFO,"%s kernel loaded\n",(char*)KERNEL_NAME);
    kdebug(DINFO,"Using %s ui system ver %lx.%lx\n",(char*)UI_SYSTEM_NAME,(uint64)UI_SYSTEM_VERSION_MAJOR,(uint64)UI_SYSTEM_VERSION_MINOR);
    initKheap(kinf->load.kheapVBase,kinf->load.kheapSize);
    initPaging();
    initGDT();
    initIDT();
    initFaultHandlers();
    initPMM(kinf);
    initTSS(stackTop);
    registerSyscall(testsys,0x0);
    //initMultitasking();
    loadModule(kinf->testELF);
    //initRamdisk((void*)kinf->load.rdptr);
    initACPI(kinf);
    initPCI();
    kdebug(DNONE,"Lapic ticks: 0x%lx\n",lapicTick);
    end:
    for(;;){asm("hlt");}
}