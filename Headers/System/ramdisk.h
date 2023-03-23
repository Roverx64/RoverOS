#pragma once
#include <stdint.h>

#define RD_FILE_MAGIC 0x5244464C //RDFL
#define RD_HEADER_MAGIC 0x52444852 //RDHR

struct ramdiskHeader{
    uint32 magic;
    uint16 files; //Number of files in the ramdisk
}__attribute__((packed));

struct ramdiskFile{
    uint32 magic;
    uint64 offset; //Offset to next file
    uint64 size; //Size of file
    uint16 flags; //Flags
}__attribute__((packed));

#define FLAG_WRITE 0x1<<0

#ifndef __STDC_HOSTED__
extern void initRamdisk(void *ptr);
#endif

#define FILE_DATA(ptr,nlen,xlen) ((void*)((uint64)ptr+sizeof(struct ramdiskFile)+nlen+xlen+2))

//General Ramdisk layout
/*
[Header]
[File]
<Name><ext>
<File data>
[File]
<Name><ext>
<File data>
...
*/