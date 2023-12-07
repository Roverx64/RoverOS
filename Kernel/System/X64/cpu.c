#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "bootinfo.h"
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include "paging.h"
#include "ramdisk.h"

void initCPUs(uint64 *bar){
    //Load bootstrap code to low memory
    kmapPage(0x0,0x0,true,false,false,false,0x0,false);
    kmapPage(KALIGN(bar),KALIGN(bar),true,false,false,false,0x0,false);
    //Get file from ramdisk
    FILE *boot = openRamdiskFile("Corebootstrap","bin");
    if(boot == NULL){kdebug(DFATAL,"No Corebootstrap file found\n"); return;}
    memcpy(0x7c00,boot->stream,boot->size);
    //Write RIP and IPI to BAR
    
}