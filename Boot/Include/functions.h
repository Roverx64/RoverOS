#pragma once

#include <efidef.h>
#include "types.h"
#include "boot.h"

//File structure for my convienence
typedef struct EFIFileS{
    uint16 *name;
    EFI_HANDLE image;
    EFI_FILE_HANDLE Volume;
    EFI_FILE_HANDLE File;
    EFI_FILE_INFO *Info;
}EFIFile;

extern EFI_STATUS fileInfo(EFIFile *file);
extern EFI_STATUS loadFile(EFIFile *file, uint64 mode, uint64 flags);
extern EFI_STATUS loadElf(EFIFile *elf);
extern EFI_STATUS getMmap(struct bootInfoS *bootInfo,uint64 ksz);
extern EFI_STATUS mapMemory(struct bootInfoS *bootInfo);