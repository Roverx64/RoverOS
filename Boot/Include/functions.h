#pragma once

#include <efidef.h>
#include "stdint.h"
#include "boot.h"

//File structure for my convienence
typedef struct EFIFileS{
    uint16 *name;
    EFI_HANDLE image;
    EFI_FILE_HANDLE Volume;
    EFI_FILE_HANDLE File;
    EFI_FILE_INFO *Info;
    uint64 mode;
    uint64 flags;
}EFIFile;

extern EFI_STATUS fileInfo(EFIFile *file);
extern EFI_STATUS loadVolume(EFIFile *file);
extern EFI_STATUS loadFile(EFIFile *file);
extern EFI_STATUS loadElf(EFIFile *elf);
extern void *getSection(uint64 size, uint64 attributes);
extern EFI_STATUS initMmap(struct bootInfoS *bootInfo);
extern EFI_STATUS mapMemory(struct bootInfoS *bootInfo);
extern void initPaging();
extern void initIDT(EFI_HANDLE ImageHandle, struct bootInfoS *bi);
extern EFI_STATUS mapPage(uintptr phys, uintptr virt, bool write, bool user, bool nx, bool alloc);

#define GS_INVALID_PTR 0xFFF