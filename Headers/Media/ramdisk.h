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
    uint16_t flags; //Flags
    uint16_t name; //Offset to name relative to this struct
    uint16_t ext; //Offset to ext relative to the struct
    uint64_t offset; //Offset to next file
    uint64_t size; //Size of file
}__attribute__((packed));

//Returned by the openRamdiskFile() func
struct rdFile{
    struct ramdiskFile *rdfl; //Header struct pointer
};

#define FLAG_WRITE 0x1<<0

#define FILE_NAME(fl) ((uint64_t)fl+fl->name)
#define FILE_EXT(fl) ((uint64_t)fl+fl->ext)
#define FILE_DATA(fl,nlen) ((void*)((uint64_t)fl+sizeof(struct ramdiskFile)+nlen+1))

#ifndef __STDC_HOSTED__
extern void initRamdisk(void *ptr);
extern struct rdFile *openRamdiskFile(char *name)
extern void closeRamdiskFile(struct rdFile *fl);
extern void *getRamdiskFileBuffer(struct rdFile *fl);
#endif

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
