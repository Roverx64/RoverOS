#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "bootinfo.h"
#include "hpet.h"
#include "paging.h"
#include "ioapic.h"
#include "cpu.h"
#include <mutex.h>

//HEPT will likely be used for only task switching or things that don't need to be extremely precise
//HPET will only be used for time tracking if no other option is available
//Currently assumes only one HPET is present

mutex_t hpetLock;
struct heptDeviceInfo hpet;

static inline void writeHPET(struct hpetTimerInfo *hpet, uint64_t reg, uint64_t data, bool preserve){
    if(preserve){data |= rlmmio((uint64_t)hpet->bar,reg);}
    wlmmio(hpet->bar,reg,data);
}

static inline uint64_t readHPET(struct hpetTimerInfo *hpet, uint64_t reg){
    return rdmmio((uint64_t)hpet->bar,reg);
}



bool setHPETTimer(uint64_t ms, uint8_t vector){
    GET_LOCK(hpetLock);
    FREE_LOCK(hpetLock);
    return false;
}

void initHPET(void *ptr){
    struct acpiHPET *acpiHpet = (struct acpiHPET*)ptr;
    uint64_t hpetn = hpet.hpetCount;
    hpet.hpets[hpetn].bar = acpiHpet->bar;
    kmapPage(KALIGN(hpet.hpets[hpetn].bar),KALIGN(hpet.hpets[hpetn].bar),PAGE_FLAG_WT|PAGE_FLAG_WRITE|PAGE_FLAG_MAKE);
    hpet.hpetCount += 1;
    hpet.hpets[hpetn].id = hpetn;
    hpet.hpets[hpetn].minimumClock = acpiHpet->minClockTick;
    //Initilize
    uint64_t capabilities = readHPET(&hpet.hpets[hpetn],HPET_GENERAL_CAPABILITIES);
    hpet.hpets[hpetn].timers = (uint8_t)HPET_GENERAL_CAPABILITIES_TIMERS(capabilities);
    hpet.hpets[hpetn].wideCounter = (uint8_t)HPET_GENERAL_CAPABILITIES_COUNT_SIZE(capabilities);
    hpet.hpets[hpetn].legacyRoute = (uint8_t)HPET_GENERAL_CAPABILITIES_LEG_ROUTE(capabilities);
    hpet.hpets[hpetn].clockPeriod = (uint32_t)HPET_GENERAL_CAPABILITIES_CLOCK_PER(capabilities);
    //Initilize HPET to known values
    for(uint16_t i = 0; i < hpet.hpets[hpetn].timers; ++i){
        uint64_t timerOffset = HPET_TIMER_BASE_OFFSET(i);
        //Enable 64 bit wide counter if supported
        if(hpet.hpets[hpetn].wideCounter){
            //Done this way since Qemu's HPET doesn't work correctly
            writeHPET(&hpet.hpets[hpetn],HPET_TIMER_CONFIG_OFFSET+timerOffset,HPET_TIMER_32MODE_CNF,true);
            uint64_t q = readHPET(&hpet.hpets[hpetn],HPET_TIMER_CONFIG_OFFSET+timerOffset);
            q ^= HPET_TIMER_32MODE_CNF;
            writeHPET(&hpet.hpets[hpetn],HPET_TIMER_CONFIG_OFFSET+timerOffset,q,false);
        }
        //Clear counter
        writeHPET(&hpet.hpets[hpetn],HPET_TIMER_COMPAR_OFFSET+timerOffset,0x0,false);
    }
    kdebug(DINFO,"Intitlized HPET\n");
}
