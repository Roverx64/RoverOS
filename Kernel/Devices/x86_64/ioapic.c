#include <stdint.h>
#include <sysinf.h>
#include <ioapic.h>
#include <kernel.h>
#include <kmalloc.h>
#include "acpi.h"
#include "kprint.h"
#include "paging.h"
#include "cpu.h"
#include "lapic.h"
#include "pit.h"

size_t ioapicCount = 0;
struct ioapicInfo *ioapic;

extern struct icsHeader *findNextICS(uint8_t type, struct icsHeader *current, bool checkInitial);

//Abstractions are still here due to how strange accessing these registers is

//TODO: Do not assume there is only one ioapic

uint32_t readIOAPIC(uint32_t reg){
    ioapic[0].mmio->regsel = reg;
    return ioapic[0].mmio->win;
}

void writeIOAPIC(uint64_t val, uint32_t reg, bool qword){
    ioapic[0].mmio->regsel = reg;
    ioapic[0].mmio->win = (uint32_t)(val&0xFFFFFFFF);
    if(!qword){return;}
    ++reg;
    ioapic[0].mmio->regsel = reg;
    ioapic[0].mmio->win = (uint32_t)(val>>32);
}

//Returns an 8 bit entry number
//0xFF will be returned on failure
uint8_t allocateIOAPICInterrupt(uint8_t vector, uint8_t mode, uint8_t apicid){
    if((vector < 0x10) || (vector > 0xFE)){return 0xFF;}
    //Allocate tbl entry
    uint32_t entry = 0;
    for(; entry <= 24; ++entry){
        if((ioapic[0].intmap&(1<<entry)) == 0){break;}
    }
    if(entry == 24){return 0xFF;}
    ioapic[0].intmap |= 1<<entry;
    //Write entry
    uint32_t reg = IOAPIC_REDTBL_ENTRY(entry);
    uint64_t value = IOAPIC_REDTBL_APICID(apicid)|IOAPIC_REDTBL_DELIVER_MODE(mode);
    value |= IOAPIC_REDTBL_VECTOR(vector);
    writeIOAPIC(value,reg,true);
    kinfo("Set IOAPIC entry 0x%x to route to 0x%x\n",(uint32_t)entry,vector);
    return entry;
}
/*!
    Allocates specific IOAPIC interrupt vector
    Returns 0xFF on failure.
*/
uint8_t setIOAPICInterrupt(uint8_t apicVector, uint8_t intVector, uint8_t mode, uint8_t apicid){
    if((intVector < 0x10) || (intVector > 0xFE)){kwarn("Interrupt vector out of range\n"); return 0xFF;}
    //Allocate tbl entry
    if(ioapic[0].intmap&(0x1<<apicVector) != 0x0){kwarn("IOAPIC vector 0x%x is in use\n",apicVector); return 0xFF;}
    ioapic[0].intmap |= (1<<apicVector);
    //Write entry
    uint32_t reg = IOAPIC_REDTBL_ENTRY(apicVector);
    uint64_t value = IOAPIC_REDTBL_APICID(apicid)|IOAPIC_REDTBL_DELIVER_MODE(mode);
    value |= IOAPIC_REDTBL_VECTOR(intVector);
    writeIOAPIC(value,reg,true);
    kinfo("Set IOAPIC entry 0x%x to route to 0x%x\n",(uint32_t)apicVector,(uint32_t)intVector);
    return apicVector;
}

extern volatile intHandler pitInterrupt(registers r);

void initIOAPIC(void *ptr, struct icsHeader *headers){
    struct ioApic *apic = (struct ioApic*)ptr;
    if(IS_UNALIGNED(apic->ioAddr)){
        kwarn("bad IOAPIC at (0x%lx) with phys (0x%lx)\n",(uint64_t)ptr,(uint64_t)apic->ioAddr);
        return;
    }
    if(ioapic){
        void *np = krealloc(ioapic,sizeof(struct ioapicInfo)*(ioapicCount+1));
        KASSERT((np != NULL),"failed to realloc for ioapic");
        ioapic = (struct ioapicInfo*)np;
    }
    else{
        ioapic = (struct ioapicInfo*)kmalloc(sizeof(struct ioapicInfo));
        KASSERT((ioapic != NULL),"NULL ioapic struct");
    }
    ioapic[ioapicCount].physBAR = apic->ioAddr;
    kinfo("IOAPIC at 0x%lx\n",(uint64_t)ioapic[ioapicCount].physBAR);
    uint64_t virt = (uint64_t)vmallocPhys(KALIGN(ioapic[ioapicCount].physBAR),sizeof(struct ioapicMMIO),VTYPE_MMIO,VFLAG_WT|VFLAG_WRITE|VFLAG_MAKE);
    ioapic[ioapicCount].mmio = (struct ioapicMMIO*)virt;
    KASSERT(((uint64_t)ioapic[ioapicCount].mmio != 0x0),"NULL ioapic MMIO");
    ioapic[ioapicCount].intmap = 0x0;
    sysinf.ioapic = &ioapic;
    ++ioapicCount;
    kinfo("Mapped IOAPIC 0x%lx->0x%lx\n",ioapic[ioapicCount].physBAR,virt);
    //Route PIT to CPU
    uint8_t pitMap = 0x0;
    //Check for PIT ISO
    struct icsHeader *iso = headers;
    while((iso = findNextICS(ICS_ISO,iso,false)) != NULL){
        struct interruptSourceOverride *is = (struct interruptSourceOverride*)iso;
        if(is->source != PIT_CHANNEL0){continue;}
        kinfo("Found PIT override 0x%x->0x%x\n",(uint32_t)PIT_CHANNEL0,is->gsi);
        pitMap = is->gsi;
        break;
    }
    uint8_t icd = setIOAPICInterrupt(pitMap,32,IOAPIC_DELIVERY_MODE_FIXED,0x0); //BSP core
    if(icd == 0xFF){kwarn("Failed to set interrupt for PIT\n");}
}
