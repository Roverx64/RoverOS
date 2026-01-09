#include <stdint.h>
#include <bootinfo.h>
#include <kprint.h>
#include <kernel.h>
#include <mmap.h>

char *mmapTypeString[MMAP_TYPE_MAX+1] = {
    "IGNORE","FREE", "UEFI", "VESA",
    "MMIO", "ACPI", "RESV",
    "UNUSABLE", "KERNEL", "OTHER",
    "ELF", "PAGE", "BOOT",
    "PMM"
};

uint64_t entries = 0;
uint64_t aentries = 0;
mmapEntry *mmap = NULL;
mmapEntry *allocMap = NULL;

/*!
    !D Intilizes mmap.c's variables.
    !I kinf: Kernel bootInfo structure
    !R None
    !C NOTE:  This must be called before any other mmap.c function
*/
void initMmap(struct bootInfo *kinf){
    mmap = (mmapEntry*)kinf->mem.ptr;
    KASSERT((mmap != NULL),"Bootloader failed to pass valid mmap");
    entries = kinf->mem.entries;
    aentries = kinf->mem.aentries;
    allocMap = (mmapEntry*)kinf->mem.aptr;
}

/*!
    !D Returns a pointer to the mmap
    !R Starting mmapEntry
*/
mmapEntry *getMmap(){
    return mmap;
}

/*!
    !D Returns an mmap entry for a given phys address.
    !I phys: Physical address to search for
    !I alloc: True if this should search the allocMap over mmap
    !R mmapEntry on success, NULL if entry doesn't exist.
    !C NOTE: Callers should use wrappers defined in the mmap.h header over calling this directly
*/
mmapEntry *getMmapEntry_internal(uint64_t phys,bool alloc){
    mmapEntry *map = mmap;
    uint64_t limit = entries;
    if(alloc){map = allocMap; uint64_t limit = aentries;}
    for(uint64_t i = 0; i < limit; ++i){
        if(map[i].type == MMAP_TYPE_IGNORE){continue;}
        if((map[i].phys <= phys) && ((map[i].phys+map[i].bytes) >= phys)){
            return &map[i];
        }
    }
    return NULL;
}

/*!
    !D Prints the mmap for debugging
    !R None
*/
void dumpMmap(){
    mmapEntry *prm = mmap;
    rePrint:
    for(uint64_t i = 0; i < entries; ++i){
        if(prm[i].type == MMAP_TYPE_IGNORE){continue;}
        if(prm[i].magic != MMAP_MAGIC){knone("[BAD MAGIC]");}
        knone("[%s]",mmapTypeString[prm[i].type]);
        if(prm[i].flags&MMAP_FLAG_ALIGN){knone("[A");}else{knone("[-");}
        if(prm[i].flags&MMAP_FLAG_PARSED){knone("P]");}else{knone("-]");}
        knone("[0x%lx->0x%lx][0x%lx bytes]\n",prm[i].phys,prm[i].phys+prm[i].bytes,prm[i].bytes);
    }
    if(prm == allocMap){return;}
    prm = allocMap;
    goto rePrint;
}