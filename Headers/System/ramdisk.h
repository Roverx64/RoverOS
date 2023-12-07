#pragma once
#include <stdint.h>

#define RD_FILE_MAGIC 0x5244464C //RDFL
#define RD_HEADER_MAGIC 0x52444852 //RDHR

struct ramdiskHeader{
    uint32_t magic;
    uint16_t files; //Number of files in the ramdisk
}__attribute__((packed));

struct ramdiskFile{
    uint32_t magic;
    uint64_t offset; //Offset to next file
    uint64_t size; //Size of file
    uint16_t flags; //Flags
    uint16_t name; //Offset to name relative to this struct
    uint16_t ext; //Offset to ext relative to the struct
}__attribute__((packed));

#define FLAG_WRITE 0x1<<0

#ifndef __STDC_HOSTED__
extern void initRamdisk(void *ptr);
#endif

#define FILE_NAME(ptr) ((uint64)ptr+ptr->name)
#define FILE_EXT(ptr) ((uint64)ptr+ptr->ext)
#define FILE_DATA(ptr,nlen,xlen) ((void*)((uint64_t)ptr+sizeof(struct ramdiskFile)+nlen+xlen+2))

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