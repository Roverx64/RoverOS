#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "bootinfo.h"

void initHPET(void *ptr){
    struct acpiHPET *hpet = (struct acpiHPET*)ptr;
    kdebug(DNONE,"#===============HPET===============#\n");
    kdebug(DNONE,"EVTB: 0x%lx\n",hpet->eventTimerBlockID);
    kdebug(DNONE,"lb0: 0x%x\n",hpet->lowerBase0);
    kdebug(DNONE,"lb1: 0x%lx\n",hpet->lowerBase1);
    kdebug(DNONE,"HPET #%hx\n",hpet->hpetNum);
    kdebug(DNONE,"MinClock: 0x%x\n",(uint32)hpet->minClockTick);
    kdebug(DNONE,"PAT: 0x%hx\n",hpet->pgAttribute);
    //Initilize
}