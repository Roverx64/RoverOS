#include <efi.h>
#include <efilib.h>
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

EFI_FILE_HANDLE Vol;
EFI_FILE_HANDLE Kernel;
uint64 ksize;
const uint16 *kname = L"RoverOS.bin";

//This was a pain to get working
EFI_STATUS getKernel(EFI_HANDLE image){
    EFI_LOADED_IMAGE *ldImg = NULL;
    EFI_GUID ldGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *io;
    EFI_FILE_HANDLE Volume;
    uefi_call_wrapper(BS->HandleProtocol,3,image,&ldGuid,(void**)&ldImg);
    uefi_call_wrapper(BS->HandleProtocol,3,ldImg->DeviceHandle,&fsGuid,(void*)&io);
    Volume = LibOpenRoot(ldImg->DeviceHandle);
    uefi_call_wrapper(Volume->Open,5,Volume,&Kernel,kname,EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    EFI_FILE_INFO *fl = LibFileInfo(Kernel);
    Print(L"Kernel Size: 0x%llx\n",fl->FileSize);
    ksize = (uint64)fl->FileSize;
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
        if(entry->Type == EfiConventionalMemory && GET_ENTRY_SZ(entry->NumberOfPages,EFI_PAGE_SIZE) >= ksize){
            bootInfo.memory.baseLoad = entry->PhysicalStart;
            bootInfo.memory.baseVload = ROVEROS_BASE_LOAD;
            Print(L"Using 0x%llx->0x%llx for phys | 0x%llx for virt | Size: 0x%llx\n",(uint64)entry->PhysicalStart,(uint64)entry->PhysicalStart+(uint64)GET_ENTRY_SZ(entry->NumberOfPages,EFI_PAGE_SIZE),(uint64)bootInfo.memory.baseVload,(uint64)GET_ENTRY_SZ(entry->NumberOfPages,EFI_PAGE_SIZE));
            break;
        }
        entry = NextMemoryDescriptor(entry,bootInfo.memory.mmapDescriptorSize);
    }
    //Check if a valid entry was found
    if(bootInfo.memory.baseLoad == NULL){Print(L"No suitable memory found\n"); return !EFI_SUCCESS;}
    return EFI_SUCCESS;
}

EFI_STATUS copyKernel(){
    //Map address being copied to
    uefi_call_wrapper(ST->BootServices->AllocatePages,4,AllocateAddress,EfiConventionalMemory,(UINTN)(ksize/EFI_PAGE_SIZE)+1,bootInfo.memory.baseLoad);
    //Read kernel to base load address
    uefi_call_wrapper(Kernel->Read,3,Kernel,&ksize,(uint8*)bootInfo.memory.baseLoad);
    Print(L"Copied kernel to memory\n");
    //Map memory
    uefi_call_wrapper(ST->BootServices->AllocatePages,4,AllocateAddress,EfiConventionalMemory,(UINTN)(ksize/EFI_PAGE_SIZE)+1,(EFI_PHYSICAL_ADDRESS)bootInfo.memory.baseVload);
    Print(L"Mapped pages\n");
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
//Main functions
*/
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
    InitializeLib(ImageHandle,SystemTable);
    if(StrCmp(ST->FirmwareVendor,L"EDK II\0") == 0){uefi_call_wrapper(ST->ConOut->ClearScreen,1,ST->ConOut);}
    Print(L"RoverOS EFI Loaded\n");
    printInfo();
    SystemTable->BootServices->SetWatchdogTimer = (uint64)30;
    SystemTable->BootServices->SetWatchdogTimer = 0;
    //Get info to load RoverOS kernel
    if(getKernel(ImageHandle) != EFI_SUCCESS){goto efiErrorEnd;}
    if(getMmap() != EFI_SUCCESS){goto efiErrorEnd;}
    if(getACPI() != EFI_SUCCESS){goto efiErrorEnd;}
    if(copyKernel() != EFI_SUCCESS){goto efiErrorEnd;}
    SystemTable->BootServices->SetWatchdogTimer = 0;
    Print(L"Disabled watchdog\n");
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