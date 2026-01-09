#include <stdint.h>
#include <mutex.h>
#include <port.h>
#include <cpuid.h>
#include <cpu.h>
#include <ramdisk.h>
#include <kmalloc.h>
#include <string.h>
#include <pit.h>
#include "acpi.h"
#include "lapic.h"
#include "paging.h"
#include "kprint.h"
#include "kernel.h"
#include <vmm.h>
#include <pmm.h>
#include <sysinf.h>

extern void lapicIRQ(void);
extern bool hasDeadline(void);
struct lapicInfo *lapic;
uint16_t apInit = 0;
uint16_t apCount = 0;
mutex_t lapicLock = 0;
extern pageSpace kspace;

static void disablePIC(){
    outb(0xA1, 0xFF);
    outb(0x21, 0xFF);
}

static inline uint32_t readLapic(uint16_t apn, uint64_t reg){
    if(lapic[apn].x2apic){return (uint32_t)rdmsr(LAPIC_MSR_BASE+reg);}
    return rdmmio((uint64_t)lapic[apn].physBAR,reg);
}

static inline void writeLapic(uint16_t apn, uint64_t reg, uint64_t data, bool preserve){
    if(lapic[apn].x2apic){
        if(preserve){data |= rdmsr(LAPIC_MSR_BASE+reg);}
        wrmsr(LAPIC_MSR_BASE+reg,data);
        return;
    }
    if(preserve){data |= rdmmio((uint64_t)lapic[apn].physBAR,reg);}
    wdmmio((uint64_t)lapic[apn].physBAR,reg,data);
}

void lapicInterrupt(){
    //Send EOI
    //writeLapic();
}

//BSP/APs call this when intilizing their lapic
//Returns apInit
uint64_t registerLapic(uint64_t bar, bool bsc){
    GET_LOCK(lapicLock);
    uint16_t apn = apInit;
    ++apInit;
    //Map bar as UC
    uint64_t virt = (uint64_t)vmallocPhys(bar,sizeof(struct lapicMMIO),VTYPE_MMIO,VFLAG_UC|VFLAG_MAKE|VFLAG_WRITE);
    lapic[apn].physBAR = bar;
    lapic[apn].mmio = (struct lapicMMIO*)virt;
    lapic[apn].bsc = bsc;
    uint32_t tst = 0;
    uint32_t trs = 0;
    uint64_t msr = 0x0;
    lapic[apn].id = LAPIC_ID_AID(lapic[apn].mmio->id);
    //Clear CCR
    lapic[apn].mmio->timerCurrentCount = 0;
    //Enable APIC
    msr = rdmsr(LAPIC_MSR_BASE);
    msr |= LAPIC_MSR_BASE_APIC_ENABLE;
    wrmsr(LAPIC_MSR_BASE,msr);
    //Check for x2APIC
    CPUID(0x1,&trs,&trs,&tst);
    if(tst&CPUID_X2APIC_PRESENT){
        lapic[apn].x2apic = true;
        //Enable x2APIC
        msr = rdmsr(LAPIC_MSR_BASE);
        msr |= LAPIC_MSR_BASE_X2APIC_ENABLE;
        wrmsr(LAPIC_MSR_BASE,msr);
        //Unmap bar
        unmapPage(&kspace,lapic[apn].physBAR);
        lapic[apn].physBAR = 0x0;
    }
    else{lapic[apn].x2apic = false;}
    //Calibrate lapic
    //Divide by 64
    writeLapic(apn,LAPIC_DIVIDE_CONFIG,LAPIC_SET_DIVIDE_CONFIG(0x5),false);
    //Start timer
    writeLapic(apn,LAPIC_INITIAL_COUNT,0xFFFFFFFF,false);
    //Wait for PIT and then read current count
    //pitCalibrate(0x71AE);
    uint32_t count = readLapic(apn,LAPIC_INITIAL_COUNT);
    uint64_t frequency = 0xFFFFFFFF-count;
    FREE_LOCK(lapicLock);
    return apn;
}
//1.193182MHz/41 = 0x71AE
//((i-c)*41)/1193182

void writeNewTaskTimer(uint16_t id, uint32_t ticks){
    writeLapic(id,LAPIC_EOI,0,false);
    writeLapic(id,LAPIC_INITIAL_COUNT,ticks,false); //Time slice
}

void initLapic(uint32_t bar, uint16_t apCount, void *wakeup){
    kinfo("Intilizing lapic\n");
    disablePIC();
    lapic = (struct lapicInfo*)kmalloc(sizeof(struct lapicInfo)*apCount);
    uint64_t ap = registerLapic((uint64_t)bar,true);
    kinfo("Initilized lapic\n");
    return;
}