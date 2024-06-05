#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "paging.h"
#include "cpu.h"
#include "lapic.h"

struct ioapicDevice ioapic;

uint64_t ioApicBAR = 0x0;

uint32_t readIOAPIC(uint32_t reg){
    uint64_t r = IOAPIC_REGSEL;
    wdmmio(ioApicBAR,r,reg);
    uint32_t v = rdmmio(ioApicBAR,IOAPIC_WIN);
    return v;
}

void writeIOAPIC(uint64_t val, uint32_t reg, bool qword){
    wdmmio(ioApicBAR,IOAPIC_REGSEL,reg);
    wdmmio(ioApicBAR,IOAPIC_WIN,val&0xFFFFFFFF);
    if(!qword){return;}
    ++reg;
    wdmmio(ioApicBAR,IOAPIC_REGSEL,reg);
    val = val>>32;
    wdmmio(ioApicBAR,IOAPIC_WIN,val);
}

//Returns an 8 bit entry number
//0xFF will be returned on failure
uint8_t allocateIOAPICInterrupt(uint8_t vector, uint8_t mode, uint8_t apicid){
    if((vector < 0x10) || (vector > 0xFE)){return 0xFF;}
    //Allocate tbl entry
    uint32_t entry = 0;
    for(; entry <= 24; ++entry){
        if((ioapic.intmap&(1<<entry)) == 0){break;}
    }
    if(entry == 24){return 0xFF;}
    ioapic.intmap |= 1<<entry;
    //Write entry
    uint32_t reg = IOAPIC_REDTBL_ENTRY(entry);
    uint64_t value = IOAPIC_REDTBL_APICID(apicid)|IOAPIC_REDTBL_DELIVER_MODE(mode);
    value |= IOAPIC_REDTBL_VECTOR(vector);
    writeIOAPIC(value,reg,true);
    return entry;
}

void initIOAPIC(void *ptr){
    struct ioApic *apic = (struct ioApic*)ptr;
    ioapic.bar = (uint64_t)apic->ioAddr;
    ioapic.intmap = 0x0;
    kmapPage(KALIGN(apic->ioAddr),KALIGN(apic->ioAddr),PAGE_FLAG_WT|PAGE_FLAG_WRITE|PAGE_FLAG_MAKE);
}
