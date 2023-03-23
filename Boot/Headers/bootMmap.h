#pragma once
#include <efi.h>
#include "boottypes.h"
#define PAGER_MINSZ 0x200000*2 //4MB should be enough
#define NEXT_DESC(ptr,sz) ((EFI_MEMORY_DESCRIPTOR*)((uint64)ptr+sz))
#define DESC_SZ(desc) (desc->NumberOfPages*EFI_PAGE_SIZE)

extern void *getMemory(uint64 size);
extern EFI_STATUS getMmap();
extern void mapMMAP();