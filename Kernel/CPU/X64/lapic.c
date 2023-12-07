#include <stdint.h>
#include "acpi.h"
#include "lapic.h"
#include "paging.h"
#include "mmio.h"
#include "debug.h"
#include "interrupt.h"
#include "kernel.h"

uint64 lbar = 0x0;
uint64 lapicTick = 0x0;
extern void lapicIRQ(void);
extern bool hasDeadline(void);

static inline uint32_t readLapic(uint64 bar, uint64 reg){
    uint32 v = 0;
    rdmmio(bar,reg,v);
    return v;
}

static inline void writeLapic(uint64 bar, uint64 reg, uint32 data, bool preserve){
    uint32 write = 0x0;
    if(preserve){rdmmio((uint64)bar,reg,write);}
    write = write|data;
    wdmmio(bar,reg,write);
}

uint64 getTick(){
    return lapicTick;
}

intHandler lapicTimer(registers reg){
    kdebug(DNONE,"LAPIC\n");
    writeLapic(lbar,LAPIC_EOI,0x1,false);
}

intHandler lapicSpurriousInt(registers reg){
    kdebug(DNONE,"Spurrious\n");
}

void setLapicInterrupts(uint64 bar){
    writeLapic(bar,LAPIC_CMCI,14,false);
    writeLapic(bar,LAPIC_THERMAL,14,false);
    writeLapic(bar,LAPIC_PERFORMANCE,14,false);
    writeLapic(bar,LAPIC_LINT0,14,false);
    writeLapic(bar,LAPIC_LINT1,14,false);
    writeLapic(bar,LAPIC_LVT_ERROR,14,false);
    writeLapic(bar,LAPIC_SPURIOUS_INT,14,true);
}

void initLapicTimer(uint64 bar, uint32 count, uint8 vector){
    writeLapic(bar,LAPIC_INITIAL_COUNT,0x0,false);
    writeLapic(bar,LAPIC_CURRENT_COUNT,0x0,false);
    //Set divide register
    writeLapic(bar,LAPIC_DIVIDE_CONFIG,0x1,false);
    //Set vector and unmask interrupts
    uint32 data = readLapic(bar,LAPIC_TIMER);
    data = data&0xFFFEFF00;
    data |= vector|LAPIC_TIMER_MODE_PERIODIC;
    writeLapic(bar,LAPIC_TIMER,data,false);
    //Set counter
    writeLapic(bar,LAPIC_INITIAL_COUNT,count,false);
}

void initDeadlineTimer(uint64 bar, uint8 vector){

}

void wakeAP(uint32 *bar, uint32 apicid){

}

void calibrateLapic(){
    
}

void setDeadline(uint64 time){
    uint64 lower = time&0xFFFFFFFF;
    uint64 upper = time>>32;
    LAPIC_SET_DEADLINE(upper,lower);
}

void startDeadline(){
    
}

void deadlineIRQ(){
    kdebug(DNONE,"DEADLINE\n");
}

void initLapic(uint32 *bar32){
    uint64 bar = (uint64)bar32;
    lbar = bar;
    kdebug(DNONE,"Initilizing lapic\n");
    kdebug(DNONE,"BAR: 0x%lx\n",(uint64)bar);
    kmapPage(KALIGN((uint64)bar),KALIGN((uint64)bar),true,false,false,false,PG_PCD,true);
    uint64 id = readLapic(bar,LAPIC_ID);
    uint64 ver = readLapic(bar,LAPIC_VER);
    kdebug(DNONE,"LAPIC 0x%lx Ver 0x%lx\n",id,ver&0xFF);
    sysinf.time.hasDeadline = hasDeadline();
    kdebug(DNONE,"TSC-Deadline: ");
    if(sysinf.time.hasDeadline){
        kdebug(DNONE,"Present\n");
        overrideIDTEntry(30,deadlineIRQ);
        initDeadlineTimer(bar,30);
        sysinf.time.deadlinetpms = 10;
        goto end;
    }
    kdebug(DNONE,"Absent\n");
    overrideIDTEntry(30,(void*)lapicIRQ);
    initLapicTimer(bar,0xFF,30);
    end:
    //Enable lapic MSR
    enableLapic();
    uint32 spurr = readLapic(bar,LAPIC_SPURIOUS_INT);
    spurr = spurr&0xFFFFFF00;
    spurr |= LAPIC_FLAG_ENABLE_LAPIC;
    setLapicInterrupts(bar);
    writeLapic(bar,LAPIC_SPURIOUS_INT,spurr,false);
}