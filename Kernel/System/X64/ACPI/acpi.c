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

void parseTable(void *ptr){
    struct acpiDescHeader *desc = (struct acpiDescHeader*)ptr;
    kdebug(DNONE,"#===============");
    kmapPage(KALIGN((uint64)&desc->signature),KALIGN((uint64)&desc->signature),false,false,false,false,0x0,true);
    printAPCIstr(&desc->signature,4);
    kdebug(DNONE,"===============#\n");
    switch(desc->signature){
        case FACP_SIGNATURE:
        initFACP(ptr);
        break;
        case HPET_SIGNATURE:
        initHPET(ptr);
        break;
        case APIC_SIGNATURE:
        initAPIC(ptr);
        break;
        default:
        kdebug(DNONE,"Note: This table is unsupported\n");
        kdebug(DNONE,"SignatureID: 0x%x\n",desc->signature);
        break;
    }
    kdebug(DNONE,"OEM: "); printAPCIstr(&desc->oemid,6);
    kdebug(DNONE,"\nCreator: "); printAPCIstr(&desc->creatorid,4);
    kdebug(DNONE,"\n");
    
}

void initACPI(struct bootInfo *kinf){
    struct rsdp *rsdp = (struct rsdp*)kinf->xsdtptr;
    kmapPage(KALIGN(kinf->xsdtptr),KALIGN(kinf->xsdtptr),false,false,false,false,0x0,true);
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
    kdebug(DNONE,"OEM: "); printAPCIstr(&rsdp->oemid,6);
    kdebug(DNONE,"\nRSDT: 0x%x\n",rsdp->rsdt);
    kdebug(DNONE,"Revision: 0x%x\n",(uint32)rsdp->revision);
    struct rsdt *rsdt = (struct rsdt*)rsdp->rsdt;
    uint32 *tables = (uint32*)(rsdp->rsdt+sizeof(struct rsdt));
    kdebug(DNONE,"Tables at 0x%lx\n",(uint64)tables);
    kmapPages(KALIGN((uint64)tables),KALIGN((uint64)tables),false,false,false,false,0x0,true,5);
    //Parse
    for(int i = 0; tables[i] != 0x0; ++i){
        parseTable((void*)tables[i]);
    }
    kdebug(DNONE,"#==================================#\n");
}