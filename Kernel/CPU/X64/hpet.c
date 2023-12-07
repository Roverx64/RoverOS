#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "bootinfo.h"
#include "hpet.h"
#include "paging.h"
#include "ioapic.h"
#include "interrupt.h"
#include "mmio.h"

uint64 tick = 0;
void *hpetbar;

void writeHPET(uint64 reg, uint64 data, bool preserve, uint64 sz){
    uint64 write = 0x0;
    if(preserve){write = readMMIO(hpetbar,reg,sz);}
    write = write|data;
    writeMMIO(hpetbar,reg,write,sz);
}

uint64 readHPET(uint64 reg, uint64 sz){
    return readMMIO(hpetbar,reg,sz);
}

intHandler timerInterrupt(registers reg){
    ++tick;
    //Clear int
    uint64 i = readHPET(HPET_GENERAL_INT_STATUS,true);
    writeHPET(HPET_GENERAL_INT_STATUS,i,false,MMIO_QWORD);
    //Reset counter
    uint64 time = readHPET(HPET_MAIN_COUNTER_VAL,MMIO_QWORD);
    time+=0x3E8;
    writeHPET(0x100+HPET_TIMER_COMPAR_OFFSET,time,false,MMIO_QWORD);
}

void initTimer(uint64 *bar, uint32 timer, bool periodic, uint64 comp, uint32 vector){
    uint32 toff = HPET_TIMER_BASE_OFFSET(timer);
    //Qemu HPET hack
    writeHPET(toff+HPET_TIMER_CONFIG_OFFSET,HPET_TIMER_MODE_32,true,MMIO_QWORD);
    uint64 q = readHPET(toff+HPET_TIMER_CONFIG_OFFSET,true);
    q = q^HPET_TIMER_MODE_32;
    writeHPET(toff+HPET_TIMER_CONFIG_OFFSET,q,false,MMIO_QWORD);
    //Enable Periodic
    //writeHPET(toff+HPET_TIMER_CONFIG_OFFSET,HPET_TIMER_TYPE_CNF|HPET_TIMER_VAL_SET,true,true,false);
    //writeHPET(toff+HPET_TIMER_COMPAR_OFFSET,0x0,false,true,false);
    //Write comparator
    writeHPET(toff+HPET_TIMER_COMPAR_OFFSET,comp,false,MMIO_QWORD);
    //Get valid route
    uint64 cfg = readHPET(toff+HPET_TIMER_CONFIG_OFFSET,true);
    cfg = HPET_TIMER_INT_ROUTE_CAP(cfg);
    uint64 b = 0;
    for(;b < 32;++b){if((cfg>>b)&0x1 == 0x1){break;}}
    //Write route in HPET
    writeHPET(toff+HPET_TIMER_CONFIG_OFFSET,HPET_TIMER_INT_ROUTE((uint64)0x0,b),true,MMIO_QWORD);
    //Set IOAPIC
    writeIOAPICInterrupt(b,vector,IOAPIC_DELIVERY_MODE_FIXED,false,false,0x0);
    //Enable interrupts
    writeHPET(toff+HPET_TIMER_CONFIG_OFFSET,HPET_TIMER_INT_ENABLE_CNF|HPET_TIMER_INT_TYPE_CNF,true,MMIO_QWORD);
    kdebug(DNONE,"T%x Comp: 0x%lx CFG: 0x%lx Route: 0x%lx\n",timer,readHPET(toff+HPET_TIMER_COMPAR_OFFSET,MMIO_QWORD),readHPET(toff+HPET_TIMER_CONFIG_OFFSET,MMIO_QWORD),b);
    uint64 tst = readHPET(toff+HPET_TIMER_CONFIG_OFFSET,MMIO_QWORD);
}

volatile void initHPET(void *ptr){
    return;
    struct acpiHPET *hpet = (struct acpiHPET*)ptr;
    kdebug(DNONE,"EVTB: 0x%lx\n",hpet->eventTimerBlockID);
    kdebug(DNONE,"BAR: 0x%lx\n",hpet->bar);
    kdebug(DNONE,"HPET #%hx\n",hpet->hpetNum);
    kdebug(DNONE,"MinClock: 0x%x\n",(uint32)hpet->minClockTick);
    kdebug(DNONE,"PAT: 0x%hx\n",hpet->pgAttribute);
    //Initilize
    kmapPage(KALIGN(hpet->bar),KALIGN(hpet->bar),true,false,false,false,0x0,false);
    uint64 *hpetMMIO = (uint64*)hpet->bar;
    hpetbar = (void*)hpetMMIO;
    uint64 capabilities = readHPET(HPET_GENERAL_CAPABILITIES,MMIO_QWORD);
    kdebug(DNONE,"Cap: 0x%lx\n",capabilities);
    uint32 revID = (uint32)HPET_GENERAL_CAPABILITIES_REV(capabilities);
    uint16 timerCap = (uint16)HPET_GENERAL_CAPABILITIES_TIMERS(capabilities);
    uint8 countCap = (uint8)HPET_GENERAL_CAPABILITIES_COUNT_SIZE(capabilities);
    uint8 routeCap = (uint8)HPET_GENERAL_CAPABILITIES_LEG_ROUTE(capabilities);
    uint16 vendorID = (uint8)HPET_GENERAL_CAPABILITIES_VENDOR_ID(capabilities);
    uint32 clockPeriod = (uint32)HPET_GENERAL_CAPABILITIES_CLOCK_PER(capabilities);
    kdebug(DNONE,"Revision 0x%x\nVendor: 0x%x\n",revID,vendorID);
    kdebug(DNONE,"Legacy route support: ");
    if(routeCap){kdebug(DNONE,"Yes\n");}else{kdebug(DNONE,"No\n");}
    kdebug(DNONE,"64 bit counter: ");
    if(countCap){kdebug(DNONE,"Yes\n");}else{kdebug(DNONE,"No\n");}
    kdebug(DNONE,"0x%x timers available\n",(uint32)timerCap+1);
    kdebug(DNONE,"Period (femt): 0x%x\n",clockPeriod);
    kdebug(DNONE,"Period (nano): 0x%x\n",clockPeriod/1000000);
    kdebug(DNONE,"Frequency: 0x%x\n",0x38d7ea4c68000/clockPeriod);
    kdebug(DNONE,"Initilizing HPET\n");
    //Clear main counter
    writeHPET(HPET_MAIN_COUNTER_VAL,0x0,false,MMIO_QWORD);
    //Setup timer
    initTimer(hpetMMIO,0,true,0x1,40);
    //Enable interrupts
    addInterruptHandler(40,timerInterrupt);
    writeHPET(HPET_GENERAL_CONFIGURATIION,HPET_GENERAL_CONFIGURATION_ENABLE_CNF,true,MMIO_QWORD);
    asm("hlt");
    kdebug(DNONE,"OK Comp: 0x%lx\n",readHPET(HPET_TIMER_BASE_OFFSET(0)+HPET_TIMER_COMPAR_OFFSET,true));
    asm("hlt");
    kdebug(DNONE,"OK2\n");
}