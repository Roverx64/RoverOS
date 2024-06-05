#pragma once
#include <efi.h>
#include <stdint.h>

typedef struct EFIFile{
    uint16_t *name;
    EFI_HANDLE image;
    EFI_FILE_HANDLE Volume;
    EFI_FILE_HANDLE File;
    EFI_FILE_HANDLE Directory;
    EFI_FILE_INFO *Info;
    uint64_t mode;
    uint64_t flags;
}EFIFile;

extern EFI_STATUS openVolume(EFIFile *file, EFI_HANDLE IH);
extern EFI_STATUS getFileInfo(EFIFile *file);
extern uint64_t getFilePos(EFIFile *file);
extern EFI_STATUS setFilePos(EFIFile *file, uint64_t pos);
extern EFI_STATUS openFile(EFIFile *file, uint16_t *name, uint64_t mode, uint64_t flags);
extern EFI_STATUS readFile(EFIFile *fl, void *dest, UINTN bytes);
extern EFI_STATUS closeFile(EFIFile *file);
extern EFI_STATUS closeVolume(EFIFile *file);