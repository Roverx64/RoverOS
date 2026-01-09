#include <stdint.h>
#include <mutex.h>
#include <kernel.h>
#include <sysinf.h>
#include <kprint.h>
#include <timer.h>
#include <hpet.h>
#include "acpi.h"
#include "bootinfo.h"
#include "paging.h"
#include "ioapic.h"
#include "cpu.h"

//HEPT will likely be used for only task switching or things that don't need to be extremely precise
//HPET will only be used for time tracking if no other option is available
//Currently assumes only one HPET is present

mutex_t hpetLock;
volatile struct hpetInfo hpet;
timerCallback callback;

/*!
    Called on timer interrupt
*/
void hpetInterrupt(){
    hpet.mmio->timer[0].compareValue = 0x0;
    callback(0);
}

/*!
    Changes the HPET specified timer to the specified time
*/
timerSet setTimer(uint64_t time, bool ns, timerCallback call, uint8_t timer){
    uint64_t t = (ns == true) ? time*0xF4240 : time*0xE8D4A51000;
    //Read current time and offset it
    uint64_t count = hpet.mmio->mainCounter;
    hpet.mmio->timer[timer].compareValue = count+time;
    callback = call;
}

/*!
    Intilized the HPET
*/
void initHPET(void *ptr){
    struct acpiHPET *acpiHpet = (struct acpiHPET*)ptr;
    hpet.mmio = (struct hpetMMIO*)vmallocPhys(acpiHpet->bar,sizeof(struct hpetMMIO),VTYPE_MMIO,VFLAG_WRITE|VFLAG_MAKE|VFLAG_WT);
    KASSERT((uint64_t)hpet.mmio,"Allocating a virtual address failed");
    hpet.physBAR = acpiHpet->bar;
    uint64_t cap = hpet.mmio->capabilities;
    hpet.timers = HPET_GENERAL_CAPABILITIES_TIMERS(cap);
    hpet.extended = (bool)HPET_GENERAL_CAPABILITIES_COUNT_SIZE(cap);
    hpet.legacyRoute = (bool)HPET_GENERAL_CAPABILITIES_LEG_ROUTE(cap);
    hpet.minClock = acpiHpet->minClockTick;
    hpet.clockPeriod = HPET_GENERAL_CAPABILITIES_CLOCK_PER(cap);
    hpet.pageProtection = acpiHpet->pageProtection;
    //Initilize HPET to known values
    uint64_t base = (uint64_t)hpet.mmio;
    for(uint16_t i = 0; i < hpet.timers; ++i){
        //Enable 64 bit wide counter if supported
        if(hpet.extended){
            //Done this way since Qemu's HPET doesn't work correctly
            hpet.mmio->timer[i].configCapability |= HPET_TIMER_32MODE_CNF;
            hpet.mmio->timer[i].configCapability ^= HPET_TIMER_32MODE_CNF;
        }
        //Clear counter
        hpet.mmio->timer[i].compareValue = 0x0;
    }
    //Only one HPET timer is supported for now
    bool ns = (hpet.clockPeriod < 0xF4240) ? true : false;
    //Registers a timer as availble for use
    registerTimer(TIMER_TYPE_HPET,setTimer);
    kinfo("Intitlized HPET\n");
}
