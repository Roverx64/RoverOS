#pragma once
#include <stdint.h>
#include <cpu.h>

#define LAPIC_MSR_BASE 0x1B
#define LAPIC_MSR_BASE_BSC 0x1<<0
#define LAPIC_MSR_BASE_X2APIC_ENABLE 0x1<<10
#define LAPIC_MSR_BASE_APIC_ENABLE 0x1<<11
#define LAPIC_MSR_BASE_APIC_BAR(msr) ((msr>>12)&0xFFFFFFFFFF)

struct lapicBAR{
    uint64_t res0; //Reserved
    uint32_t id; //Local APIC ID
    uint32_t version; //Version
    uint64_t res1[2];
    uint32_t taskPriority;
    uint32_t arbitrationPriority;
    uint32_t processerPriority;
    uint32_t EOI;
    uint32_t remoteRead;
    uint32_t logicalDestination;
    uint32_t destinationFormat;
    uint32_t spurriousIntVector;
    uint32_t isr[8];
    uint32_t tmr[8];
    uint32_t irr[8];
    uint32_t errorStatus;
}__attribute__((packed));

struct lapicDeviceInfo{
    uint16_t id;
    bool bsc;
    void *bar;
    uint64_t phys;
    bool x2apic;
};

#define LAPIC_ID 0x20
#define LAPIC_ID_AID(id) (id>>24)

static inline uint16_t getLapicID(){
    uint64_t msr = rdmsr(0x1B);
    uint16_t x2 = msr&(1<<10);
    if(x2){return LAPIC_ID_AID(rdmsr(LAPIC_MSR_BASE+LAPIC_ID));}
    uint64_t bar = msr>>12;
    return LAPIC_ID_AID(((uint32_t*)bar)[LAPIC_ID]);
}

#define LAPIC_VERSION 0x30
#define LAPIC_TPR 0x80 //Task priority register
#define LAPIC_APR 0x90 //Arbitration priority register
#define LAPIC_PPR 0xA0 //Process prority register
#define LAPIC_EOI 0xB0 //End of interrupt
#define LAPIC_RRR 0xC0 //Remote read register
#define LAPIC_LDR 0xD0 //Logical destination register
#define LAPIC_DFR 0xE0 //Destination fromat register
#define LAPIC_SPURIOUS_INT 0xF0
#define LAPIC_ISR(idx) ((idx*16)+0x100)
#define LAPIC_TMR(idx) ((idx*16)+0x180)
#define LAPIC_IRR(idx) ((idx*16)+0x200)
#define LAPIC_ESR 0x280 //Error status register
#define LAPIC_CMCI 0x2F0
#define LAPIC_ICR0 0x300 //Lower Interrupt command register
#define LAPIC_ICR1 0x310 //Upper Interrupt command register
#define LAPIC_TIMER_LVT 0x320
#define LAPIC_THERMAL_LVT 0x330
#define LAPIC_PERFORMANCE 0x340
#define LAPIC_LINT0 0x350
#define LAPIC_LINT1 0x360
#define LAPIC_LVT_ERROR 0x370
#define LAPIC_INITIAL_COUNT 0x380
#define LAPIC_CURRENT_COUNT 0x390
#define LAPIC_DIVIDE_CONFIG 0x3E0

#define LAPIC_FLAG_ENABLE_LAPIC 1<<8

#define LAPIC_TIMER_MODE_ONE_SHOT 0x0
#define LAPIC_TIMER_MODE_PERIODIC ((uint32_t)0x1<<17)
#define LAPIC_TIMER_MODE_DEADLINE ((uint32_t)0x2<<17)

#define LAPIC_ICR_VECTOR(vec) (vec)
#define LAPIC_ICR_MT_FIXED 0x0
#define LAPIC_ICR_MT_LOWEST_PRIORITY 0x1
#define LAPIC_ICR_MT_SMI 0x2
#define LAPIC_ICR_MT_REMOTE_READ 0x3
#define LAPIC_ICR_MT_NMI 0x4
#define LAPIC_ICR_MT_INIT 0x5
#define LAPIC_ICR_MT_SIPI 0x6
#define LAPIC_ICR_MT_EXTINT 0x7
#define LAPIC_ICR_DM_LOGICAL 1<<11
#define LAPIC_ICR_DM_PHYSICAL 1<<12
#define LAPIC_ICR_DS 1<<13
#define LAPIC_ICR_L_ASSERT 1<<14
#define LAPIC_ICR_L_DEASSERT 0x0
#define LAPIC_ICR_TGM_LEVEL_SENSITIVE 1<<15
#define LAPIC_ICR_TGM_EDGE_SENSITIVE 0x0
#define LAPIC_ICR_RRS(icr) ((icr>>16)&0x3)
#define LAPIC_ICR_RRS_INVALID_READ 0x0
#define LAPIC_ICR_RRS_DELIVERY_PENDING 0x1
#define LAPIC_ICR_RRS_DELIVERY_COMPLETE 0x2
#define LAPIC_ICR_DSH_DESTINATION_UNUSED 0x0
#define LAPIC_ICR_DSH_DESTINATION_SELF 0x1
#define LAPIC_ICR_DSH_DESTINATION_ALL_EXC_SELF 0x2
#define LAPIC_ICR_DSH_DESTINATION_ALL_INC_SELF 0x3
#define LAPIC_ICR_DES(des) ((uint64_t)des<<56)

extern void enableLapic(void);

#define LAPIC_SET_DEADLINE(tu,tl) asm volatile("movq %0, %%rax;movq %1, %%rdx;movq $0x6E0, %%rcx;wrmsr;"::"r" (tu), "r" (tl):"rax","rdx","rcx")