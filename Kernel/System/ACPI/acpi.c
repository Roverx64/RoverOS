#include <stdint.h>
#include "acpi.h"
#include "kprint.h"
#include "paging.h"
#include "bootinfo.h"

/*!
    !D Prints an ACPI string (table/vendor/etc)
    !I str: String to write
    !I bytes: bytes in 'str'
    !R None
*/
void printAPCIstr(uint8_t *str, uint8_t bytes){
    for(int i = 0; i < bytes; ++i){
        kprintChar((char)str[i]);
    }
}

extern void initHPET(void *ptr);
extern void initAPIC(void *ptr);

/*!
    !D Identifies an ACPI table and calls dedicated functions to parse it
    !I ptr: Pointer to table
    !R None
*/
void indentifyTable(void *ptr){
    struct acpiDescHeader *desc = (struct acpiDescHeader*)ptr;
    knone("["); printAPCIstr(&desc->signature,4);
    knone("|"); printAPCIstr(&desc->oemid,6);
    knone("|"); printAPCIstr(&desc->creatorid,4);
    knone("|0x%lx]\n",desc->signature);
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

/*!
    !D Intilizes ACPI and reads through tables
    !I kinf: Kernel bootInfo structure
    !R None
*/
void initACPI(struct bootInfo *kinf){
    kinfo("Parsing ACPI structures\n");
    struct rsdp *rsdp = (struct rsdp*)kinf->acpiptr;
    if(rsdp->signature != RSDP_SIGNATURE || rsdp->signature == XSDT_SIGNATURE){
        kinfo("Invalid ACPI signature 0x%llx\n",rsdp->signature);
        return;
    }
    if(rsdp->signature == XSDT_SIGNATURE){
        kinfo("Found valid XSDT (unsupported) signature\n");
        kinfo("Using RSDP instead\n");
    }
    else{kinfo("Found valid RSDP signature\n");}
    knone("#===============ACPI===============#\n");
    knone("Revision: 0x%x\n",(uint32)rsdp->revision);
    struct rsdt *rsdt = (struct rsdt*)rsdp->rsdt;
    uint32_t *tables = (uint32_t*)(rsdp->rsdt+sizeof(struct rsdt));
    //Parse
    for(int i = 0; tables[i] != 0x0; ++i){
        indentifyTable((void*)tables[i]);
    }
    knone("#==================================#\n");
}