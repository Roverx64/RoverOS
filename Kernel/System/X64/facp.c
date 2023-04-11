#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "bootinfo.h"

void initFACP(void *ptr){
    struct acpiFACP *facp;
    kdebug(DNONE,"#===============FACP===============#\n");
    //Causes a #GP
    kdebug(DNONE,"FCTRL: 0x%x\n",0x0);//facp->firmwareCTRL);
}