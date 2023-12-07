#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "bootFile.h"

EFI_STATUS openVolume(EFIFile *file, EFI_HANDLE IH){
    EFI_LOADED_IMAGE *ldImg = NULL;
    EFI_GUID ldGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *io;
    file->image = IH;
    uefi_call_wrapper(BS->HandleProtocol,3,file->image,&ldGuid,(void**)&ldImg);
    uefi_call_wrapper(BS->HandleProtocol,3,ldImg->DeviceHandle,&fsGuid,(void*)&io);
    file->Volume = LibOpenRoot(ldImg->DeviceHandle);
    return EFI_SUCCESS;
}

EFI_STATUS getFileInfo(EFIFile *file){
    file->Info = LibFileInfo(file->File);
    return EFI_SUCCESS;
}

uint64 getFilePos(EFIFile *file){
    uint64 pos = 0x0;
    EFI_STATUS status = uefi_call_wrapper(file->File->GetPosition,2,file->File,&pos);
    return pos;
}

EFI_STATUS setFilePos(EFIFile *file, uint64 pos){
    EFI_STATUS status = uefi_call_wrapper(file->File->SetPosition,2,file->File,pos);
    if(EFI_ERROR(status)){return status;}
    return EFI_SUCCESS;
}

EFI_STATUS openFile(EFIFile *file, uint16 *name, uint64 mode, uint64 flags){
    file->name = name;
    file->flags = flags|EFI_FILE_SYSTEM;
    file->mode = mode;
    EFI_STATUS status = uefi_call_wrapper(file->Volume->Open,5,file->Volume,&file->File,file->name,file->mode,file->flags);
    if(status == EFI_NOT_FOUND){Print(L"FILE  : Failed to find file\n"); return !EFI_SUCCESS;}
    if(status == EFI_NO_MEDIA){Print(L"FILE  : No medium found\n"); return !EFI_SUCCESS;}
    if(status == EFI_OUT_OF_RESOURCES){Print(L"FILE  : Not enough resources to open file\n"); return !EFI_SUCCESS;}
    return EFI_SUCCESS;
}

EFI_STATUS readFile(EFIFile *fl, void *dest, UINTN bytes){
    uefi_call_wrapper(fl->File->Read,3,fl->File,&bytes,dest);
    return EFI_SUCCESS;
}

EFI_STATUS closeFile(EFIFile *file){
    uefi_call_wrapper(file->File->Close,1,file->File);
    return EFI_SUCCESS;
}

EFI_STATUS closeVolume(EFIFile *file){
    uefi_call_wrapper(file->Volume->Close,1,file->Volume);
}