#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "bootinfo.h"

void initFACP(void *ptr){
    struct acpiFACP *facp = (struct acpiFACP*)ptr;
    kdebug(DNONE,"FCTRL: 0x%x\n",facp->firmwareCTRL);
}