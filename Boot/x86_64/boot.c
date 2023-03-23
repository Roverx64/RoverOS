#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "bootGop.h"
#include "bootMmap.h"
#include "bootPaging.h"

struct bootInfo kinf;
typedef (*kfunc)(struct bootInfo*);

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
    //Init
    EFI_STATUS status = EFI_SUCCESS;
    InitializeLib(ImageHandle,SystemTable);
    Print(L"RoverOS loaded\n");
    Print(L"Using %s\n",ST->FirmwareVendor);
    status = findBestMode();
    if(status != EFI_SUCCESS){goto error;}
    //Memory
    if(getMmap() != EFI_SUCCESS){goto error;}
    if(initPaging() != EFI_SUCCESS){goto error;}
    //ACPI
    Print(L"Getting xdst\n");
    EFI_GUID acpi = AcpiTableGuid;
    status = LibGetSystemConfigurationTable(&acpi,(void**)&kinf.xdstptr);
    if(EFI_ERROR(status)){Print(L"Failed to get xdst\n"); goto error;}
    Print(L"Got xdst at 0x%llx\n",(uint64)kinf.xdstptr);
    //Setup kernel
    uint64 entry = bootstrapKernel(ImageHandle);
    if(entry == 0x0){goto error;}
    //Cleanup/jump
    //initConsole();
    Print(L"Preparing to jump to 0x%llx\n",entry);
    UINTN key = 0x0;
    UINTN trash;
    (void*)LibMemoryMap(&trash,&key,&trash,(UINT32*)&trash);
    status = uefi_call_wrapper(ST->BootServices->ExitBootServices,2,ImageHandle,key);
    if(status == EFI_INVALID_PARAMETER){Print(L"Failed to exit boot services\n"); goto error;}
    setCR3();
    kinf.magic = BOOTINFO_MAGIC;
    kfunc jump = (kfunc)entry;
    //Set stack
    jump(&kinf);
    //Loop forever
    for(;;){asm("hlt");}
    error:
    initConsole();
    Print(L"Press any key to return to UEFI\n");
    Pause();
    return status;
}