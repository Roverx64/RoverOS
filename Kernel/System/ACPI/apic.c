#include <stdint.h>
#include <acpi.h>
#include <lapic.h>
#include <kprint.h>
#include <paging.h>
#include <kmalloc.h>
#include <kernel.h>

extern void initIOAPIC(void *ptr, struct icsHeader *headers);
extern void initLapic(uint32_t bar, uint16_t apCount, void *wakeup);

uint64_t madtBase = 0;
uint32_t madtLength = 0;

/*!
    !D Finds next ICS header of type relative to 'current'.
    !I type: Type of header
    !I current: Current header
    !I checkInitial: Skips checking 'current' if true
    !R Returns icsHeader or NULL if none is found.
*/
struct icsHeader *findNextICS(uint8_t type, struct icsHeader *current, bool checkInitial){
    uint64_t clen = ((uint64_t)current-madtBase)-sizeof(struct acpiMADT);
    uint32_t i = 0;
    struct icsHeader *header = NULL;
    struct icsHeader *check = current;
    if(checkInitial){
        if(check->type == type){return check;}
    }
    while(clen <= madtLength){
        check = (struct icsHeader*)((uint64_t)check+check->length);
        clen += check->length;
        if(check->type == type){
            header = check;
            break;
        }
    }
    return header;
}

/*!
    !D Initilizes APICS and LAPICs
    !I ptr: Pointer to the MADT
    !R None
*/
void initAPIC(void *ptr){
    struct acpiMADT *madt = (struct acpiMADT*)ptr;
    madtBase = (uint64_t)ptr;
    madtLength = madt->length;
    uint16_t apCount = 1;
    kinfo("Parsing MADT (0x%lx)\n",(uint64_t)ptr);
    //Read ICS structures for IOApic and APs
    struct icsHeader *header = (struct icsHeader*)((uint64_t)ptr+sizeof(struct acpiMADT));
    kinfo("Parsing ICS structures for at 0x%lx\n",(uint64_t)header);
    kinfo("Checking for IOAPICS\n");
    struct icsHeader *check = header;
    struct icsHeader *mpw = NULL;
    check = findNextICS(ICS_IOAPIC,check,true);
    while(check != NULL){
        initIOAPIC(check,header);
        check = findNextICS(ICS_IOAPIC,check,false);
    }
    check = findNextICS(ICS_IOAPIC,header,true);
    if(check != NULL){
        mpw = check;
        kinfo("Found MP wakeup\n");
    }
    else{kinfo("No MP wakeup found\n");}
    check = findNextICS(ICS_IOAPIC,header,true);
    while((check = findNextICS(ICS_LAPIC,check,false)) != NULL){
        struct lapic *lp = (struct lapic*)check;
        if((lp->flags&LAPIC_ENABLED)|(lp->flags&LAPIC_ONLINE_CAPABLE) == 0x0){continue;}
        ++apCount;
    }
    kinfo("Enumerated 0x%x usable APs and BSP\n",(uint32_t)apCount);
    initLapic((uint32_t)madt->LocalInterruptControllerAddress,apCount,mpw);
}