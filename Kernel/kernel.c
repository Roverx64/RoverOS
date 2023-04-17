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
    kdebug(DINFO,"Using %s ui system\n",(char*)UI_SYSTEM_NAME);
    initWindow(&kwindow,kinf->ui.Xres,kinf->ui.Yres);
    kwindow.buffer = (UI_BUFFER*)kinf->ui.framebuffer;
    kwindow.objects = NULL;
    //Test text obj
    text txt;
    initText(&kwindow,&txt,0xFFFFFF,2,2,30,30,10,10);
    drawString(&kwindow,&txt,&testFont,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    initPMM(kinf);
    initRamdisk((void*)kinf->load.rdptr);
    initACPI(kinf);
    end:
    for(;;){asm("hlt");}
}