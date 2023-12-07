#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "bootinfo.h"
#include "lapic.h"
#include "kheap.h"

extern bool hasX2APIC(void);
extern void enableX2APIC(void);

const char *icsNames[24] = {
    "Local APIC\0", "IO APIC\0",
    "Interrupt Source Override\0", "NMI\0",
    "Local APIC NMI", "Local APIC Address Override\0",
    "Source APIC\0", "Local Source APIC\0",
    "Platform Interrupt Source\0", "x2APIC\0",
    "GIC CPU Interface\0", "GIC Distributor\0",
    "GIC MSI Frame\0", "GIC Redistributor\0",
    "GIC Interrupt Translation Service\0", "Multiprocessor Wakeup\0",
    "CORE PIC\0", "Legacy PIC\0",
    "HyperTransport PIC\0", "Legacy PIC\0",
    "Extended PIC\0", "MSI PIC\0",
    "Bridge PIC\0", "Low Pin PIC\0"
};

uint64 APcount = 0;
uint64 **lapics = 0x0;

void parseLapic(void *ptr){
    struct lapic *lp = (struct lapic*)ptr;
    lapics[lp->lapicID] = (uint64*)ptr;
    kdebug(DNONE,"- UID: 0x%hx - Lapic: 0x%hx",lp->uid,lp->lapicID);
    if(lp->flags&LAPIC_ENABLED){kdebug(DNONE," - Ready]\n"); goto inc;}
    if(lp->flags&LAPIC_ONLINE_CAPABLE){kdebug(DNONE," - Waiting]\n"); goto inc;}
    kdebug(DNONE," - Unusable]\n");
    return;
    inc:
    ++APcount;
}

void parseIOApic(void *ptr){
    struct ioApic *ioa = (struct ioApic*)ptr;
    kdebug(DNONE,"- ID: 0x%hx - Addr: 0x%x - SIB: 0x%x]\n",ioa->id, ioa->ioAddr,ioa->sib);
    initIOAPIC(ioa);
}

void parseISO(void *ptr){
    struct interruptSourceOverride *iso = (struct interruptSourceOverride*)ptr;
    kdebug(DNONE,"- Bus: 0x%hx - Source: 0x%hx - GSI: 0x%x]\n",iso->bus,iso->source,iso->gsi);
}

void parseLapicNMI(void *ptr){
    struct lapicNMI *lp = (struct lapicNMI*)ptr;
    kdebug(DNONE,"- UID: 0x%hx - Lint: 0x%hx]\n",lp->uid,lp->lint);
}

void initAPIC(void *ptr){
    struct acpiMADT *madt = (struct acpiMADT*)ptr;
    lapics = halloc(sizeof(uint64)*8);
    kdebug(DNONE,"Revision: 0x%hx\n",madt->revision);
    kdebug(DNONE,"Lapic BAR: 0x%x\n",madt->LocalInterruptControllerAddress);
    kdebug(DNONE,"Flags: 0x%x\n",madt->flags);
    uint64 icsPtr = (uint64)ptr+sizeof(struct acpiMADT);
    struct icsHeader *header = (struct icsHeader*)icsPtr;
    kdebug(DNONE,"x2Apic: ");
    if(hasX2APIC()){
        kdebug(DNONE,"Yes\n");
        //enableX2APIC();
        kdebug(DNONE,"Enabled x2Apic mode\n");
    }else{kdebug(DNONE,"No\n");}
    for(uint64 sz = sizeof(struct acpiMADT);sz < madt->length;icsPtr += header->length){
        header = (struct icsHeader*)icsPtr;
        sz += header->length;
        if(header->type >= 23){continue;}
        kdebug(DNONE,"[%s ",icsNames[header->type]);
        switch(header->type){
            case LAPIC:
            parseLapic((void*)icsPtr);
            break;
            case IOAPIC:
            parseIOApic((void*)icsPtr);
            break;
            case ISO:
            parseISO((void*)icsPtr);
            break;
            case LAPIC_NMI:
            parseLapicNMI((void*)icsPtr);
            break;
            default:
            kdebug(DNONE,"- Len: 0x%hx - unsupported]\n",header->length);
            break;
        }
    }
    //Load flat binary coreboot to 0x7c00
    //memcpy(0x7c00,boot->corebootSz,boot->coreboot);

    //Setup lapic and intilize APs
    initLapic((uint32*)madt->LocalInterruptControllerAddress);
    //Setup APs
    if(APcount == 0x1){return;}
    //kdebug(DNONE,"Initilizing 0x%x APs\n",APcount-1);
    //struct lapic *lp = (struct lapic*)lapics[1];
    //wakeAP((uint32*)madt->LocalInterruptControllerAddress,lp->lapicID);
}