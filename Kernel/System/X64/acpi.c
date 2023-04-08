#include <stdint.h>
#include "acpi.h"
#include "debug.h"
#include "bootinfo.h"

bool checkSignature(void *sig, void* check){
    uint8 *c1 = (uint8*)sig;
    uint8 *c2 = (uint8*)check;
    for(int i = 0; i < 8; ++i){
        kdebug(DNONE,"%c=%c\n",(int)c1[i],(int)c2[i]);
        if(c1[i] != c2[i]){return false;}
    }
    kdebug(DNONE,"\n");
    return true;
}

void initACPI(struct bootInfo *kinf){
    struct rdsp *rsdp = (struct rsdp*)kinf->xdstptr;
    if(rsdp->signature != ACPI_SIGNATURE){
        kdebug(DINFO,"Invalid ACPI signature 0x%llx\n",rsdp->signature);
    }
    kdebug(DINFO,"Found valid ACPI signature\n");
}