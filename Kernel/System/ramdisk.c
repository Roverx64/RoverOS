#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <libpng.h>
#include "ramdisk.h"
#include "kheap.h"

//For reading files from the ramdisk
struct ramdiskHeader *header = NULL;

FILE *openRamdiskFile(const char *name, const char *ext){
    struct ramdiskFile *fl = (struct ramdiskFile*)((uint64)header+sizeof(struct ramdiskHeader));
    bool found = false;
    for(uint16 i = 0; i < header->files; ++i){
        if((strcmp(name,(char*)FILE_NAME(fl)) == 0) || (strcmp(ext,(char*)FILE_EXT(fl)) == 0)){
            found = true;
            break;
        }
    }
    if(!found){return NULL;}
    FILE *rdfl = halloc(sizeof(FILE));
    rdfl->stream = FILE_DATA(fl,strlen((char*)FILE_NAME(fl)),strlen((char*)FILE_EXT(fl)));
    rdfl->offset = 0;
    rdfl->size = fl->size;
}

void initRamdisk(void *ptr){
    header = (struct ramdiskHeader*)ptr;
    if(header->magic != RD_HEADER_MAGIC){
        kdebug(DERROR,"Invalid ramdisk header\n");
        return;
    }
}