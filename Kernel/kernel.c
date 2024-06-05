#include <stdint.h>
#include "bootinfo.h"
#include <debug.h>
#include <kernel.h>
#include "ramdisk.h"
#include "sysinf.h"
#include "pmm.h"
#include "cpu.h"
#include "paging.h"
#include "syscall.h"

struct systemInformation sysinf;
extern void initBSPGDT(void);
extern void initTasking();
extern void initKmalloc(struct bootInfo *kinf);
extern void initPaging();
extern void initACPI(struct bootInfo *kinf);
extern void initPCI();

void kmain(struct bootInfo *kinf){
    if(kinf->magic != BOOTINFO_MAGIC){
        kdebug(DERROR,"Invalid bootinfo magic\n");
        goto end;
    }
    kdebug(DINFO,"%s kernel loaded\n",(char*)KERNEL_NAME);
    //kdebug(DINFO,"Using %s ui system ver %lx.%lx\n",(char*)UI_SYSTEM_NAME,(uint64_t)UI_SYSTEM_VERSION_MAJOR,(uint64_t)UI_SYSTEM_VERSION_MINOR);
    initBSPGDT();
    initIDT();
    initPMM(kinf);
    initPaging();
    initKmalloc(kinf);
    //initTasking();
    initACPI(kinf);
    initPCI();
    end:
    for(;;){asm("hlt");}
}
