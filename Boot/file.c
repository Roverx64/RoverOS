#include <efi.h>
#include <efilib.h>
#include "types.h"
#include "functions.h"

EFI_STATUS fileInfo(EFIFile *file){
    file->Info = LibFileInfo(file->File);
    return EFI_SUCCESS;
}

EFI_STATUS loadFile(EFIFile *file, uint64 mode, uint64 flags){
    EFI_LOADED_IMAGE *ldImg = NULL;
    EFI_GUID ldGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *io;
    uefi_call_wrapper(BS->HandleProtocol,3,file->image,&ldGuid,(void**)&ldImg);
    uefi_call_wrapper(BS->HandleProtocol,3,ldImg->DeviceHandle,&fsGuid,(void*)&io);
    file->Volume = LibOpenRoot(ldImg->DeviceHandle);
    uefi_call_wrapper(file->Volume->Open,5,file->Volume,&file->File,file->name,mode,flags);
    return EFI_SUCCESS;
}