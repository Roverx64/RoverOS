#include <stdint.h>
#include <acpi.h>
#include <lapic.h>
#include <debug.h>
#include <paging.h>
#include <kmalloc.h>

extern void initIOAPIC(void *ptr);
extern void initLapic(uint32_t bar, uint16_t apCount, void *wakeup);

void initAPIC(void *ptr){
    struct acpiMADT *madt = (struct acpiMADT*)ptr;
    uint16_t apCount = 0;
    kdebug(DINFO,"Parsing MADT\n");
    //Read ICS structures for IOApic and APs
    struct icsHeader *mpWakeup = NULL;
    struct icsHeader *header = (struct icsHeader*)((uint64_t)madt+sizeof(struct acpiMADT));
    uint64_t len = sizeof(struct acpiMADT);
    for(int i = 0;len < madt->length;++i){
        len += header->length;
        switch(header[i].type){
            case ICS_IOAPIC:
            initIOAPIC(&header[i]);
            break;
            case ICS_LAPIC:
            ++apCount;
            break;
            case ICS_MP_WAKEUP:
            mpWakeup = &header[i];
            break;
            default:
            continue;
            break;
        }
    }
    //Initilize lapic
    initLapic((uint32_t)madt->LocalInterruptControllerAddress,apCount,mpWakeup);
}