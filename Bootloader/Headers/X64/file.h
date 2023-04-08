#pragma once
#include <efi.h>
#include "boottypes.h"

typedef struct EFIFile{
    uint16 *name;
    EFI_HANDLE image;
    EFI_FILE_HANDLE Volume;
    EFI_FILE_HANDLE File;
    EFI_FILE_INFO *Info;
    uint64 mode;
    uint64 flags;
}EFIFile;

extern EFI_STATUS loadVolume(EFIFile *file, EFI_HANDLE image);
extern EFI_STATUS fileLoad(EFIFile *file, uint16 *name, uint64 mode, uint64 flags);
extern EFI_STATUS fileRead(EFIFile *fl, void *dest, UINTN bytes);
extern EFI_STATUS fileClose(EFIFile *fl);
extern EFI_STATUS fileSet(EFIFile *file, uint64 pos);
extern uint64 fileGet(EFIFile *file);