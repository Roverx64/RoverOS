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
    size_t size; //Size of file
}__attribute__((packed));

typedef struct ramdiskFile RDFL;

#define RD_FILE_DATA(fl,nlen) ((void*)((uint64_t)fl+sizeof(struct ramdiskFile)+nlen+1))
#define RD_FILE_NAME(fl) ((char*)((uint64_t)fl+sizeof(RDFL)))
#define RD_NEXT_FILE(fl,fsz,nlen) ((uint64_t)fl+fsz+nlen+1+sizeof(struct ramdiskFile))

#ifdef __STDC_HOSTED__
extern RDFL *rdOpen(char *name);
extern char *rdName(RDFL *fl);
extern void rdRead(RDFL *fl, void *d, size_t bytes);
extern size_t rdLen(RDFL *fl);
#endif