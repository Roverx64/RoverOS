#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "paging.h"
#include "mmio.h"
#include "lapic.h"

uint64 ioApicBAR = 0x0;

uint32 readIOAPIC(uint32 reg){
    uint64 r = IOAPIC_REGSEL;
    wdmmio(ioApicBAR,r,reg);
    uint32 v = 0;
    rdmmio(ioApicBAR,IOAPIC_WIN,v);
    return v;
}

void writeIOAPIC(uint64 val, uint32 reg, bool qword){
    uint64 r = IOAPIC_REGSEL;
    uint64 w = IOAPIC_WIN;
    wdmmio(ioApicBAR,r,reg);
    uint32 lower = val&0xFFFFFFFF;
    wdmmio(ioApicBAR,w,lower);
    if(!qword){return;}
    ++reg;
    wdmmio(ioApicBAR,r,reg);
    val = val>>32;
    wdmmio(ioApicBAR,w,val);
}

void writeIOAPICInterrupt(uint8 source, uint8 vector, uint8 mode, bool edge, bool logical, uint8 dest){
    uint32 reg = IOAPIC_REDTBL_ENTRY(source);
    uint64 entry = IOAPIC_REDTBL_APICID(dest);
    if(logical){entry |= IOPAIC_REDTBL_DEST_LOGICAL;}
    if(!edge){entry |= IOAPIC_REDTBL_TRIGGER_LEVEL;}
    entry |= IOAPIC_REDTBL_DELIVER_MODE(mode);
    entry |= IOAPIC_REDTBL_VECTOR(vector);
    writeIOAPIC(entry,reg,true);
}

void initIOAPIC(struct ioApic *apic){
    ioApicBAR = (uint64)apic->ioAddr;
    kmapPage(KALIGN(apic->ioAddr),KALIGN(apic->ioAddr),true,false,false,false,0x0,false);
    disablePIC();
}