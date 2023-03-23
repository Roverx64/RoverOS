#include <stdint.h>
#include <string.h>
#include <debug.h>
#include "gdt.h"

struct gdtPointer gdtp;
struct gdtEntry gdtEntries[8];

extern void loadGDT(void *ptr);

void writeGDT(struct gdtEntry *entries, uint32 n, bool conforming, bool execute, bool type, uint8 dpl, bool present, bool lm){
    entries[n].conforming = conforming;
    entries[n].execute = execute;
    entries[n].type = type;
    entries[n].dpl = dpl;
    entries[n].present = present;
    entries[n].lm = lm;
}

void initGDT(){
    gdtp.limit = (sizeof(struct gdtEntry)*8)-1;
    gdtp.base = (uint64)&gdtEntries;
    kdebug(DINFO,"[GDTP|Limit: 0x%llx|Base: 0x%llx]\n",(uint64)gdtp.limit,gdtp.base);
    memset(&gdtEntries,0x0,sizeof(struct gdtEntry)*8);
    //NULL
    writeGDT(&gdtEntries,0,0,0,0,0,0,0);
    //KCode
    writeGDT(&gdtEntries,1,false,true,1,0,true,true);
    //KData
    writeGDT(&gdtEntries,2,false,false,1,0,true,true);
    //UCode
    writeGDT(&gdtEntries,3,false,true,1,0x3,true,true);
    //UData
    writeGDT(&gdtEntries,4,false,false,1,0x3,true,true);
    //TSS
    kdebug(DINFO,"Wrote GDT descriptors\n");
    loadGDT(&gdtp);
    kdebug(DINFO,"Loaded GDT\n");
}

/*
1001101 //Code
1001001 //Data
*/

//Code
//IGN       |L|IGN  |P|DP|t|x|c|IGN
//0000000000|1|00000|1|00|1|1|0|000000000000000000000000000000000000000000
//Data
//IGN       |L|IGN  |P|DP|t|x|c|IGN
//0000000000|1|00000|1|00|1|0|0|000000000000000000000000000000000000000000