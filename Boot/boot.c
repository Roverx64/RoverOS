#include <efi.h>
#include <efilib.h>
#include <efishell.h>
#include <efifs.h>
#include "types.h"
#include "boot.h"

//Setup enviornment and load the RoverOS kernel

EFI_STATUS status = 0;
struct bootInfoS bootInfo;

/*
//Misc functions
*/

void printInfo(){
    Print(L"Running on %s verison 0x%lx - 0x%llx\n",ST->FirmwareVendor,(uint32)ST->FirmwareRevision,(uint64)ST->Hdr.Signature);
}

/*
//File System Utils
*/

EFI_FILE_HANDLE GetVol(EFI_HANDLE img){
    EFI_LOADED_IMAGE *loadedImg = NULL;
    EFI_GUID guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *io;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_HANDLE Vol;
    uefi_call_wrapper(ST->BootServices->HandleProtocol,3,img,&guid,&loadedImg);
    uefi_call_wrapper(ST->BootServices->HandleProtocol,3,loadedImg->DeviceHandle,&fsGuid,&io);
    Vol = LibOpenRoot(loadedImg->DeviceHandle);
    return Vol;
}

EFI_STATUS getKernel(EFI_HANDLE image){
    return EFI_SUCCESS;
}

/*
//Memory Utils
*/

EFI_STATUS getMmap(){
    Print(L"Obtaining mmap\n");
    UINTN mmapDescVer = 0;
    bootInfo.memory.mmap = (uintptr*)LibMemoryMap((UINTN*)&bootInfo.memory.mmapSize,(UINTN*)&bootInfo.memory.mmapKey,(UINTN*)&bootInfo.memory.mmapDescriptorSize,(UINT32*)&mmapDescVer);
    Print(L"Obtained mmap version 0x%lx at 0x%llx\n",mmapDescVer,(uint64)bootInfo.memory.mmap);
    EFI_MEMORY_DESCRIPTOR *entry = (EFI_MEMORY_DESCRIPTOR*)bootInfo.memory.mmap;
    while((uint64)entry < (uint64)bootInfo.memory.mmap+(bootInfo.memory.mmapSize*bootInfo.memory.mmapDescriptorSize)){
        /*0 will be replaced with the RoverOS.bin size soon*/
        if(entry->Type == EfiConventionalMemory && GET_ENTRY_SZ(entry->NumberOfPages,EFI_PAGE_SIZE) >= 0){
            bootInfo.memory.baseLoad = entry->PhysicalStart;
            bootInfo.memory.baseVload = ROVEROS_BASE_LOAD;
            Print(L"Using 0x%llx->0x%llx for phys | 0x%llx for virt | Size: 0x%llx\n",(uint64)entry->PhysicalStart,(uint64)entry->PhysicalStart+(uint64)GET_ENTRY_SZ(entry->NumberOfPages,EFI_PAGE_SIZE),(uint64)bootInfo.memory.baseVload,(uint64)GET_ENTRY_SZ(entry->NumberOfPages,EFI_PAGE_SIZE));
            uefi_call_wrapper(ST->BootServices->AllocatePages,4,AllocateAddress,EfiConventionalMemory,(UINTN)entry->NumberOfPages,(EFI_PHYSICAL_ADDRESS)bootInfo.memory.baseVload);
            Print(L"Mapped pages\n");
            break;
        }
        entry = NextMemoryDescriptor(entry,bootInfo.memory.mmapDescriptorSize);
    }
    
    return EFI_SUCCESS;
}
/*
//ACPI Utils
*/

EFI_STATUS getACPI(){
    Print(L"Getting ACPI\n");
    EFI_GUID acpiGuid = AcpiTableGuid;
    status = LibGetSystemConfigurationTable(&acpiGuid,(void*)&bootInfo.acpi.xsdt);
    if(EFI_ERROR(status)){Print(L"EFI error getting ACPI\n"); return !EFI_SUCCESS;}
    Print(L"Got ACPI at 0x%llx\n",bootInfo.acpi.xsdt);
    return EFI_SUCCESS;
}

/*
//VESA Utils (Should only be used when transferring control to the C kernel)
*/

EFI_STATUS switchVesa(){
    return EFI_SUCCESS;
}

/*
//Main functions
*/
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
    InitializeLib(ImageHandle,SystemTable);
    if(StrCmp(ST->FirmwareVendor,L"EDK II\0") == 0){uefi_call_wrapper(ST->ConOut->ClearScreen,1,ST->ConOut);}
    Print(L"RoverOS EFI Loaded\n");
    printInfo();
    SystemTable->BootServices->SetWatchdogTimer = 0;
    Print(L"Disabled watchdog\n");
    //Get info to load RoverOS kernel
    if(getMmap() != EFI_SUCCESS){goto efiErrorEnd;}
    if(getKernel(ImageHandle) != EFI_SUCCESS){goto efiErrorEnd;}
    if(getACPI() != EFI_SUCCESS){goto efiErrorEnd;}
    if(switchVesa() != EFI_SUCCESS){goto efiErrorEnd;}
    Print(L"Exiting boot services\n");
    uefi_call_wrapper(ST->BootServices->ExitBootServices,2,ImageHandle,bootInfo.memory.mmapKey);
    Print(L"Booting RoverOS\n");
    //Loop forever on return
    asm("cli");
    for(;;){asm("hlt");}
    efiErrorEnd:
    Print(L"Press any key to continue\n");
    Pause();
    return status;
}