#include <stdint.h>
#include <string.h>
#include <debug.h>
#include <ramdisk.h>
#include <hashmap.h>
#include <kernel.h>
#include <kmalloc.h>
#include "bootinfo.h"
//For reading files from the ramdisk
struct ramdiskHeader *header = NULL;
hashmap *fmap;

//Returns a pointer to the file header
struct rdFile *openRamdiskFile(char *name){
    struct ramdiskFile *fl = (struct ramdiskFile*)hashmapGetValue(fmap,name,true);
    if(fl == NULL){return NULL;}
    struct rdFile *rd = (struct rdFile*)kmalloc(sizeof(struct rdFile));
    rd->rdfl = fl;
    return rd;
}

void closeRamdiskFile(struct rdFile *fl){
    kfree(fl);
}

void *getRamdiskFileBuffer(struct rdFile *fl){
    return (void*)FILE_DATA(fl->rdfl,strlen((char*)FILE_NAME(fl->rdfl)));
}

size_t getRamdiskFileLen(struct rdFile *fl){
    return fl->rdfl->size;
}

void initRamdisk(struct bootInfo *kinf){
    header = (struct ramdiskHeader*)kinf->rdptr;
    if(header == NULL){kpanic("NULL ramdisk ptr",0);}
    if(header->magic != RD_HEADER_MAGIC){kpanic("Invalid ramdisk magic",header->magic);}
    fmap = hashmapCreate(header->files);
    struct ramdiskFile *file = (struct ramdiskFile*)((uint64_t)header+sizeof(struct ramdiskHeader));
    //Loop through files and hash them
    for(int i = 0; i < header->files; ++i){
        if(file->magic != RD_FILE_MAGIC){kdebug(DWARN,"Invalid file magic\n"); return;}
        char *name = (char*)FILE_NAME(file);
        hashmapInsert(fmap,name,file,true);
    }
}

/*

root = /

//Theoretical directories
//Note: all programs will be sandboxed to their directories unless granted permission to other directories

/user/<username>/
               +/programs/<program name>/
                                        +/data
                                        +/bin
                                        +/documents
               +/documents
               +/media
               +/downloads
/sys/
   +/bin/
        +/boot
        +/drivers/
                 +/<driver>.kdm
        +/libs
        +/cmd
        +/headers
   +/resources/
             +/media
             +/fonts
   +/sandbox/
            +/<program name>/
                            +/permissions.lst
                            +/modify.lst
   +/programs/
             +/<program name>/
/dev/
    +/storage/
            +/<st#>
    +/serial/
            +/<sr#>
    +/PCI/
         +/<gpu#>
         +/xHCI
         +/aHCI
    +/Misc/
          +/<cpu#>
          +/<HPET#>
          +/<lapic#>

*/
