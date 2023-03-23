#include <stdint.h>
#include <string.h>
#include <debug.h>
#include <libpng.h>
#include "ramdisk.h"

//For reading files from the ramdisk
struct ramdiskHeader *header = NULL;

void initRamdisk(void *ptr){
    header = (struct ramdiskHeader*)ptr;
    if(header->magic != RD_HEADER_MAGIC){
        kdebug(DERROR,"Invalid ramdisk header\n");
        return;
    }
    //Test read first file
    struct ramdiskFile *file = (struct ramdiskFile*)((uint64)ptr+sizeof(struct ramdiskHeader));
    char *name = (char*)((uint64)file+sizeof(struct ramdiskFile));
    char *ext = (char*)((uint64)name+strlen(name)+1);
    kdebug(DINFO,"File: %s.%s\n",name,ext);
    void *data = FILE_DATA(file,strlen(name),strlen(ext));
    //Test draw png
    readPNG(data);
}