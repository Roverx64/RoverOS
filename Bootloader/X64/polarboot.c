#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "bootPaging.h"
#include "mmap.h"
#include "bootMmap.h"

struct bootInfo kinf;
typedef (*kfunc)(struct bootInfo*);
extern EFI_STATUS initSerial();
extern EFI_STATUS initHardware();
bool wait = true;

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
    InitializeLib(ImageHandle,SystemTable);
    Print(L"Polarboot loaded\n");
    EFI_LOADED_IMAGE *image = NULL;
    uefi_call_wrapper(SystemTable->BootServices->HandleProtocol,3,ImageHandle,&LoadedImageProtocol,(void**)&image);
    Print(L"BOOT  : Loaded to 0x%lx\n",(uint64)image->ImageBase);
    (void)initSerial(); //This can fail without issues
    if(initHardware() != EFI_SUCCESS){goto error;}
    if(initPaging() != EFI_SUCCESS){goto error;}
    if(initGOP() != EFI_SUCCESS){goto error;}
    uint64 kernelEntry = bootstrapKernel(ImageHandle);
    if(kernelEntry == 0x0){goto error;}
    if(setupKheap() != EFI_SUCCESS){goto error;}
    if(EFI_ERROR(LibGetSystemConfigurationTable(&AcpiTableGuid,(void**)&kinf.xsdtptr))){Print(L"BOOT  : Failed to get XSDT\n"); goto error;}
    Print(L"BOOT  : XSDT at 0x%lx\n",(uint64)kinf.xsdtptr);
    if(initMMAP() != EFI_SUCCESS){goto error;}
    mapPage(KALIGN(&kinf),KALIGN(&kinf),true,false,false);
    Print(L"BOOT  : Preparing to jump to 0x%lx\n",kernelEntry);
    UINTN key;
    UINTN trash;
    if(LibMemoryMap(&trash,&key,&trash,(UINT32*)&trash) == NULL){Print(L"BOOT  : Failed to get mmap key\n"); goto error;}
    EFI_STATUS status = uefi_call_wrapper(SystemTable->BootServices->ExitBootServices,2,ImageHandle,key);
    if(EFI_ERROR(status)){return !EFI_SUCCESS;}
    kinf.magic = BOOTINFO_MAGIC;
    kfunc jump = (kfunc)kernelEntry;
    setCR3();
    jump(&kinf);
    error:
    Print(L"Press any key to return to UEFI\n");
    Pause();
    return !EFI_SUCCESS;
}

//add-symbol-file ./Other/Polarboot.sym 0xBDF21000 -s .data 0xBDF2C000