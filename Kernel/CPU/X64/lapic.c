#include <stdint.h>
#include <mutex.h>
#include <port.h>
#include <cpuid.h>
#include <cpu.h>
#include <ramdisk.h>
#include <kmalloc.h>
#include <string.h>
#include "acpi.h"
#include "lapic.h"
#include "paging.h"
#include "debug.h"
#include "kernel.h"
#include <vmm.h>
#include <pmm.h>

extern void lapicIRQ(void);
extern bool hasDeadline(void);
struct lapicDeviceInfo *lapic;
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
    return rdmmio((uint64_t)lapic[apn].bar,reg);
}

static inline void writeLapic(uint16_t apn, uint64_t reg, uint64_t data, bool preserve){
    if(lapic[apn].x2apic){
        if(preserve){data |= rdmsr(LAPIC_MSR_BASE+reg);}
        wrmsr(LAPIC_MSR_BASE+reg,data);
        return;
    }
    if(preserve){data |= rdmmio((uint64_t)lapic[apn].bar,reg);}
    wdmmio((uint64_t)lapic[apn].bar,reg,data);
}

//BSP/APs call this when intilizing their lapic
//Returns apInit
uint64_t registerLapic(uint64_t bar, bool bsc){
    GET_LOCK(lapicLock);
    //Map bar as UC
    kmapPage(bar,bar,PAGE_FLAG_UC|PAGE_FLAG_MAKE|PAGE_FLAG_WRITE);
    lapic[apInit].bar = (void*)bar;
    lapic[apInit].phys = bar;
    lapic[apInit].bsc = bsc;
    uint32_t tst = 0;
    uint32_t trs = 0;
    uint64_t msr = 0x0;
    lapic[apInit].id = LAPIC_ID_AID(readLapic(apInit,LAPIC_ID));
    //Clear CCR
    writeLapic(apInit,LAPIC_CURRENT_COUNT,0,false);
    wrmsr(0xCCCC,0xDEADBEEFABABFCFC);
    //Enable APIC
    msr = rdmsr(LAPIC_MSR_BASE);
    msr |= LAPIC_MSR_BASE_APIC_ENABLE;
    wrmsr(LAPIC_MSR_BASE,msr);
    //Check for x2APIC
    CPUID(0x1,0x0,&trs,&trs,&trs,&tst);
    if(tst&CPUID_X2APIC_PRESENT){
        lapic[apInit].x2apic = true;
        //Enable x2APIC
        msr = rdmsr(LAPIC_MSR_BASE);
        msr |= LAPIC_MSR_BASE_X2APIC_ENABLE;
        wrmsr(LAPIC_MSR_BASE,msr);
        //Unmap bar
        //kunmapPage();
        lapic[apInit].bar = NULL;
    }
    else{lapic[apInit].x2apic = false;}
    ++apInit;
    //Calibrate lapic
    FREE_LOCK(lapicLock);
    return apInit-1;
}

void writeNewTaskTimer(uint16_t id, uint32_t ticks){
    writeLapic(id,LAPIC_EOI,0,false);
    writeLapic(id,LAPIC_INITIAL_COUNT,ticks,false); //Time slice
}

void initLapic(uint32_t bar, uint16_t apCount, void *wakeup){
    disablePIC();
    lapic = (struct lapicDeviceInfo*)kmalloc(sizeof(struct lapicDeviceInfo)*apCount);
    uint64_t ap = registerLapic((uint64_t)bar,true);
    kdebug(DINFO,"Initilized lapic\n");
    //Load Coreboot from ramdisk and load to low mem
    /*struct rdFile *coreboot = openRamdiskFile("coreboot.bin");
    uint64_t phys = allocPhys(ORDER_MIN_BLOCK_SIZE,MEMORY_TYPE_1MB,ORDER_FLAG_STRICT_MATCH);
    kmapPage(phys,phys,PAGE_FLAG_MAKE|PAGE_FLAG_WRITE|PAGE_FLAG_UC);
    memcpy((void*)phys);
    //Start up other APs
    uint32_t wakeup = LAPIC_ICR_VECTOR(0);
    wakeup |= LAPIC_ICR_MT_SIPI|LAPIC_ICR_DSH_DESTINATION_ALL_EXC_SELF;
    writeLapic(ap,LAPIC_ICR0,wakeup,false);*/
    //Wait for APs to wakeup
    //while(apInit != apCount){}
    return;
}