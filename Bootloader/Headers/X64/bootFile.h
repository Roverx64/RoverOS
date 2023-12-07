#pragma once
#include <efi.h>
#include "boottypes.h"

typedef struct EFIFile{
    uint16 *name;
    EFI_HANDLE image;
    EFI_FILE_HANDLE Volume;
    EFI_FILE_HANDLE File;
    EFI_FILE_HANDLE Directory;
    EFI_FILE_INFO *Info;
    uint64 mode;
    uint64 flags;
}EFIFile;

extern EFI_STATUS openVolume(EFIFile *file, EFI_HANDLE IH);
extern EFI_STATUS getFileInfo(EFIFile *file);
extern uint64 getFilePos(EFIFile *file);
extern EFI_STATUS setFilePos(EFIFile *file, uint64 pos);
extern EFI_STATUS openFile(EFIFile *file, uint16 *name, uint64 mode, uint64 flags);
extern EFI_STATUS readFile(EFIFile *fl, void *dest, UINTN bytes);
extern EFI_STATUS closeFile(EFIFile *file);
extern EFI_STATUS closeVolume(EFIFile *file);