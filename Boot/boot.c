#include <efi.h>
#include <efilib.h>
#include "types.h"
#include "boot.h"
#include "functions.h"

//Setup enviornment and load the RoverOS kernel

EFI_STATUS status = 0;
struct bootInfoS bootInfo;

void printInfo(){
    Print(L"Running on %s verison 0x%lx - 0x%llx\n",ST->FirmwareVendor,(uint32)ST->FirmwareRevision,(uint64)ST->Hdr.Signature);
}

EFIFile Kernel;

EFI_STATUS loadKernel(EFI_HANDLE image){
    uint16 *n = L"RoverOS.bin";
    Kernel.name = n;
    Kernel.image = image;
    loadVolume(&Kernel);
    if(status != EFI_SUCCESS){Print(L"Failed to open volume\n"); return !EFI_SUCCESS;}
    Kernel.mode = EFI_FILE_MODE_READ;
    Kernel.flags = EFI_FILE_READ_ONLY|EFI_FILE_SYSTEM;
    status = loadFile(&Kernel);
    if(status != EFI_SUCCESS){Print(L"Failed to load kernel binary\n"); return status;}
    status = fileInfo(&Kernel);
    if(status != EFI_SUCCESS){Print(L"Failed to get kernel info\n"); return status;}
    Print(L"Kernel size: 0x%llx\n",(uint64)Kernel.Info->FileSize);
    status = loadElf(&Kernel);
    return status;
}

EFI_STATUS getACPI(){
    Print(L"Getting ACPI\n");
    EFI_GUID acpiGuid = AcpiTableGuid;
    status = LibGetSystemConfigurationTable(&acpiGuid,(void*)&bootInfo.acpi.xsdt);
    if(EFI_ERROR(status)){Print(L"EFI error getting ACPI\n"); return !EFI_SUCCESS;}
    Print(L"Got ACPI at 0x%llx\n",bootInfo.acpi.xsdt);
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
    InitializeLib(ImageHandle,SystemTable);
    if(StrCmp(ST->FirmwareVendor,L"EDK II\0") == 0){uefi_call_wrapper(ST->ConOut->ClearScreen,1,ST->ConOut);}
    Print(L"RoverOS EFI Loaded\n");
    printInfo();
    SystemTable->BootServices->SetWatchdogTimer = 0;
    Print(L"Disabled watchdog\n");
    //Get info to load RoverOS kernel
    if(initMmap(&bootInfo) != EFI_SUCCESS){goto efiErrorEnd;}
    /*IDT is broken for now*/
    //initIDT(ImageHandle,&bootInfo);
    initPaging();
    if(loadKernel(ImageHandle) != EFI_SUCCESS){goto efiErrorEnd;}
    if(getACPI() != EFI_SUCCESS){goto efiErrorEnd;}
    efiErrorEnd:
    //Print(L"EOF Halting\n");
    Print(L"Press any key to return to UEFI\n");
    Pause();
    //for(;;){asm("hlt");}
    return status;
}