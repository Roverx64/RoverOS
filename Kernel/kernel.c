#include <stdint.h>
#include "bootinfo.h"
#include <kprint.h>
#include <kernel.h>
#include <kcache.h>
#include <lienzo.h>
#include <vfs.h>
#include "sysinf.h"
#include "pmm.h"
#include "cpu.h"
#include "paging.h"
#include "syscall.h"
#include <pointer.h>

extern void bootstrapCPU();
extern void initTasking();
extern void initTimers();
extern void initACPI(struct bootInfo *kinf);
extern void initPCI();
extern void initPIT();
extern void initVFS(struct bootInfo *kinf);

struct systemInformation sysinf;

/*!
    !D Main kernel entry point
    !I kinf: Kernel info structure passed from the bootloader
    !R This function never returns
*/
void kmain(struct bootInfo *kinf){
    KASSERT((kinf->magic == BOOTINFO_MAGIC),"Invalid bootinfo magic");
    kinfo("%s kernel loaded\n",(char*)KERNEL_NAME);
    bootstrapCPU();
    initMmap(kinf);
    initPMM(kinf);
    initTimers();
    initTasking(); //TSS is created here
    initPIT();
    initACPI(kinf); //x86_64 SMP is done here
    initVFS(kinf);
    initLienzo(kinf);
    lienzoWindow *win = lienzoCreateWindow(kinf->ui.Xres,kinf->ui.Yres,(rgba){0xFF,0xFF,0xFF,0xFF},0);
    drawLienzoWindow(win);
    swapLienzoBuffer();
    //vFILE *fl = openFile("/R/test.png",0);
    //lienzoLoadImage(win,fl);
    //initPCI();
    //initUnits(); //Load kernel libraries
    //Load drivers
    for(;;){asm("hlt");}
}