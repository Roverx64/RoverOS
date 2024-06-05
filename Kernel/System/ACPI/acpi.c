#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "paging.h"
#include "bootinfo.h"

void printAPCIstr(uint8 *str, uint8 bytes){
    for(int i = 0; i < bytes; ++i){
        kprintChar((char)str[i]);
    }
}

extern void initHPET(void *ptr);
extern void initAPIC(void *ptr);

void indentifyTable(void *ptr){
    struct acpiDescHeader *desc = (struct acpiDescHeader*)ptr;
    kdebug(DNONE,"["); printAPCIstr(&desc->signature,4);
    kdebug(DNONE,"|"); printAPCIstr(&desc->oemid,6);
    kdebug(DNONE,"|"); printAPCIstr(&desc->creatorid,4);
    kdebug(DNONE,"|0x%lx]\n",desc->signature);
    switch(desc->signature){
        case HPET_SIGNATURE:
        initHPET(ptr);
        break;
        case APIC_SIGNATURE: //MADT
        initAPIC(ptr);
        default:
        break;
    }

}

void initACPI(struct bootInfo *kinf){
    kdebug(DINFO,"Parsing ACPI structures\n");
    struct rsdp *rsdp = (struct rsdp*)kinf->xsdtptr;
    if(rsdp->signature != RSDP_SIGNATURE || rsdp->signature == XSDT_SIGNATURE){
        kdebug(DINFO,"Invalid ACPI signature 0x%llx\n",rsdp->signature);
        return;
    }
    if(rsdp->signature == XSDT_SIGNATURE){
        kdebug(DINFO,"Found valid XSDT (unsupported) signature\n");
        kdebug(DINFO,"Using RSDP instead\n");
    }
    else{kdebug(DINFO,"Found valid RSDP signature\n");}
    kdebug(DNONE,"#===============ACPI===============#\n");
    kdebug(DNONE,"Revision: 0x%x\n",(uint32)rsdp->revision);
    struct rsdt *rsdt = (struct rsdt*)rsdp->rsdt;
    uint32 *tables = (uint32*)(rsdp->rsdt+sizeof(struct rsdt));
    //Parse
    for(int i = 0; tables[i] != 0x0; ++i){
        indentifyTable((void*)tables[i]);
    }
    kdebug(DNONE,"#==================================#\n");
}