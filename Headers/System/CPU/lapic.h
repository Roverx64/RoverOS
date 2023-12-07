#pragma once
#include <stdint.h>

struct lapicBAR{
    uint64 res0; //Reserved
    uint32 id; //Local APIC ID
    uint32 version; //Version
    uint64 res1[2];
    uint32 taskPriority;
    uint32 arbitrationPriority;
    uint32 processerPriority;
    uint32 EOI;
    uint32 remoteRead;
    uint32 logicalDestination;
    uint32 destinationFormat;
    uint32 spurriousIntVector;
    uint32 isr[8];
    uint32 tmr[8];
    uint32 irr[8];
    uint32 errorStatus;
}__attribute__((packed));

#define LAPIC_ID 0x20
#define LAPIC_VER 0x30
#define LAPIC_TPR 0x80
#define LAPIC_APR 0x90
#define LAPIC_PPR 0xA0
#define LAPIC_EOI 0xB0
#define LAPIC_RRD 0xC0
#define LAPIC_LOGICAL_DEST 0xD0
#define LAPIC_DEST_FORMAT 0xE0
#define LAPIC_SPURIOUS_INT 0xF0
#define LAPIC_ISR(idx) ((idx*16)+0x100)
#define LAPIC_TMR(idx) ((idx*16)+0x180)
#define LAPIC_IRR(idx) ((idx*16)+0x200)
#define LAPIC_ERROR 0x280
#define LAPIC_CMCI 0x2F0
#define LAPIC_ICR0 0x300
#define LAPIC_ICR1 0x310
#define LAPIC_TIMER 0x320
#define LAPIC_THERMAL 0x330
#define LAPIC_PERFORMANCE 0x340
#define LAPIC_LINT0 0x350
#define LAPIC_LINT1 0x360
#define LAPIC_LVT_ERROR 0x370
#define LAPIC_INITIAL_COUNT 0x380
#define LAPIC_CURRENT_COUNT 0x390
#define LAPIC_DIVIDE_CONFIG 0x3E0

#define LAPIC_FLAG_ENABLE_LAPIC 1<<8

#define LAPIC_TIMER_MODE_ONE_SHOT 0x0
#define LAPIC_TIMER_MODE_PERIODIC ((uint32)0x1<<17)
#define LAPIC_TIMER_MODE_DEADLINE ((uint32)0x2<<17)

extern void initLapic(uint32 *bar32);
extern void enableLapic(void);
extern uint64 getLapicBar(void);
extern void disablePIC(void);
extern void wakeAP(uint32 *bar, uint32 apicid);

#define LAPIC_SET_DEADLINE(tu,tl) asm volatile("movq %0, %%rax;movq %1, %%rdx;movq $0x6E0, %%rcx;wrmsr;"::"r" (tu), "r" (tl):"rax","rdx","rcx")