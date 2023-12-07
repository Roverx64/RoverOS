#include <stdint.h>
#include <string.h>
#include <debug.h>
#include "gdt.h"
#include "tss.h"

struct gdtPointer gdtp;

uint64 gdtEntries[8] __attribute__((align(0x1000)));

extern void loadGDT(void *ptr);

void writeGDTSegment(uint64 *entries, uint8 entryn, uint64 dpl, bool code){
    uint64 flags = GDT_PRESENT|GDT_LM|GDT_SYSTEM|GDT_DPL(dpl);
    if(code){flags |= GDT_EXECUTE;}
    entries[entryn] = flags;
}

void writeGDTTSS(uint64 *entries, uint8 entryn, uint64 base, uint64 dpl){
    uint64 lowerTSS = GDT_TYPE_TSS|GDT_PRESENT|GDT_DPL(dpl)|sizeof(struct TSS)|GDT_TSS_BASE0(base)|GDT_TSS_BASE1(base);
    uint64 upperTSS = GDT_TSS_BASE2(base);
    entries[entryn] = lowerTSS;
    entries[entryn+1] = upperTSS;
}

void initGDT(){
    gdtp.limit = (sizeof(uint64)*8)-1;
    gdtp.base = (uint64)&gdtEntries;
    kdebug(DINFO,"[GDTP|Limit: 0x%llx|Base: 0x%llx]\n",(uint64)gdtp.limit,gdtp.base);
    memset(&gdtEntries,0x0,sizeof(uint64)*8);
    //KCode
    writeGDTSegment(&gdtEntries,1,0,true);
    //KData
    writeGDTSegment(&gdtEntries,2,0,false);
    //UCode
    writeGDTSegment(&gdtEntries,3,3,true);
    //UData
    writeGDTSegment(&gdtEntries,4,3,false);
    kdebug(DINFO,"Wrote GDT descriptors\n");
    loadGDT(&gdtp);
    kdebug(DINFO,"Loaded GDT\n");
}