#include <sefi.h>
#include <stdint.h>
#include "boot.h"
#include "bootFile.h"

EFI_STATUS openVolume(EFIFile *file, EFI_HANDLE IH){
    EFI_GUID ldGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    file->image = IH;
    EFI_STATUS status = handleProtocol(file->image,&ldGuid,(void**)&file->ldImg);
    if(status != EFI_SUCCESS){
        return status;
    }
    status = handleProtocol(file->ldImg->DeviceHandle,&fsGuid,(void**)&file->io);
    if(status != EFI_SUCCESS){
        return status;
    }
    return file->io->OpenVolume(file->io,&file->Volume);
}

EFI_STATUS getFileInfo(EFIFile *file){
    EFI_GUID ifGuid = EFI_FILE_INFO_ID;
    UINTN sz = 0;
    EFI_STATUS status = file->File->GetInfo(file->File,&ifGuid,&sz,NULL);
    if(status != EFI_BUFFER_TOO_SMALL){sprint(L"Failed with err: 0x%lx\n",status); return status;}
    file->Info = (EFI_FILE_INFO*)allocPool(sz);
    status = file->File->GetInfo(file->File,&ifGuid,&sz,file->Info);
    if(status != EFI_SUCCESS){freePool(file->Info); return status;}
    return EFI_SUCCESS;
}

/*! Currently assumes this works without fail
*/
uint64_t getFilePos(EFIFile *file){
    uint64_t pos = 0x0;
    EFI_STATUS status = file->File->GetPosition(file->File,&pos);
    return pos;
}

EFI_STATUS setFilePos(EFIFile *file, uint64_t pos){
    EFI_STATUS status = file->File->SetPosition(file->File,pos);
    return status;
}

EFI_STATUS openFile(EFIFile *file, uint16_t *name, uint64_t mode, uint64_t flags){
    file->name = name;
    file->flags = flags|EFI_FILE_SYSTEM;
    file->mode = mode;
    EFI_STATUS status = file->Volume->Open(file->Volume,&file->File,(CHAR16*)name,mode,flags);
    if(status == EFI_NOT_FOUND){sprint(L"Failed to find file\n"); return !EFI_SUCCESS;}
    if(status == EFI_NO_MEDIA){sprint(L"No medium found\n"); return !EFI_SUCCESS;}
    if(status == EFI_OUT_OF_RESOURCES){sprint(L"Not enough resources to open file\n"); return !EFI_SUCCESS;}
    if(status != EFI_SUCCESS){sprint(L"Failed with err: 0x%lx\n",status); return !EFI_SUCCESS;}
    return EFI_SUCCESS;
}

EFI_STATUS readFile(EFIFile *fl, void *dest, UINTN bytes){
    return fl->File->Read(fl->File,&bytes,dest);
}

EFI_STATUS closeFile(EFIFile *file){
    if(file->Info){freePool(file->Info);}
    return file->File->Close(file->File);
}

EFI_STATUS closeVolume(EFIFile *file){
    return file->Volume->Close(file->Volume);
}