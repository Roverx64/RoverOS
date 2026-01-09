#pragma once
#include <sefi.h>
#include <stdint.h>

typedef struct EFIFile{
    uint16_t *name; //!< File name
    EFI_HANDLE image; //!< EFI image
    EFI_FILE_PROTOCOL *Volume; //!< EFI volume
    EFI_FILE_PROTOCOL *File; //!< File handle
    EFI_FILE_PROTOCOL *Directory; //!< EFI Directory
    EFI_FILE_INFO *Info; //!< File info
    uint64_t mode; //!< File mode
    uint64_t flags; //!< File flags
    EFI_LOADED_IMAGE_PROTOCOL *ldImg; //!< EFI protocol
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *io; //!< EFI protocol
}EFIFile;

extern EFI_STATUS openVolume(EFIFile *file, EFI_HANDLE IH);
extern EFI_STATUS getFileInfo(EFIFile *file);
extern uint64_t getFilePos(EFIFile *file);
extern EFI_STATUS setFilePos(EFIFile *file, uint64_t pos);
extern EFI_STATUS openFile(EFIFile *file, uint16_t *name, uint64_t mode, uint64_t flags);
extern EFI_STATUS readFile(EFIFile *fl, void *dest, UINTN bytes);
extern EFI_STATUS closeFile(EFIFile *file);
extern EFI_STATUS closeVolume(EFIFile *file);