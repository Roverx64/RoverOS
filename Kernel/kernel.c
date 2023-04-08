#include <stdint.h>
#include <com.h>
#include "bootinfo.h"
#include <debug.h>
#include <gui.h>
#include <kernel.h>
#include <string.h>
#include "ramdisk.h"
#include "pmm.h"
#include "idt.h"
#include "gdt.h"
#include "palloc.h"

window kwindow;
extern void loadStack();
extern void initFaultHandlers();
extern void initACPI(struct bootInfo *kinf);

void kmain(struct bootInfo *kinf){
    if(kinf->magic != BOOTINFO_MAGIC){
        kdebug(DERROR,"Invalid bootinfo magic\n");
        goto end;
    }
    loadStack();
    initGDT();
    initIDT();
    initFaultHandlers();
    kdebug(DINFO,"%s kernel loaded\n",(char*)KERNEL_NAME);
    kdebug(DINFO,"Using %s windowing system\n",(char*)WINDOW_SYSTEM_NAME);
    kwindow.buffer = (WINDOW_BUFFER*)kinf->ui.framebuffer;
    kwindow.objects = NULL;
    kwindow.Xres = (WINDOW_XRES)kinf->ui.Xres;
    kwindow.Yres = (WINDOW_YRES)kinf->ui.Yres;
    kwindow.Xscale = 2;
    kwindow.Yscale = 2;
    drawString(&kwindow,60,60,&testFont,"ABCDEFGHIJKLMN");
    initPMM(kinf);
    initRamdisk((void*)kinf->load.rdptr);
    initACPI(kinf);
    end:
    for(;;){asm("hlt");}
}