#include <sefi.h>
#include <polarboot.h>
#include "bootinfo.h"
#include "bootMem.h"

struct bootInfo kinf;

uint64_t getXSDT(EFI_SYSTEM_TABLE *SystemTable){
    EFI_GUID test = EFI_ACPI_TABLE_GUID;
    for(UINTN i = 0; i < SystemTable->NumberOfTableEntries; ++i){
        if(testGUID(&test,&SystemTable->ConfigurationTable[i].VendorGuid)){
            sprint(L"Located ACPI 2.0 table\n");
            return (uint64_t)SystemTable->ConfigurationTable[i].VendorTable;
        }
    }
    sprint(L"Failed to find ACPI table\n");
    return 0x0;
}

/*!
The EFI entry point
*/
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
    initSEFI(ImageHandle,SystemTable);
    sprint(L"Polarboot loaded\n");
    clearMem(&kinf,sizeof(kinf));
    //Locate GOP handle
    EFI_STATUS status = EFI_SUCCESS;
    initMem();
    status = initGOP();
    //Initilize kernel
    uint64_t entry = bootstrapKernel(ImageHandle);
    if(entry == 0x0){sprint(L"Failed to bootstrap kernel\n"); goto failure;}
    kfunc kernel = (kfunc)entry;
    //Get XSDT
    kinf.acpiptr = getXSDT(SystemTable);
    //Map the stack
    uint64_t stack = 0x0;
    asm volatile("movq %%rbp, %0" : "=r"(stack));
    mapPage(stack,stack,true,false);
    sprint(L"Mapped the stack at 0x%lx\n",stack);
    //Map kinf
    mapPage(KALIGN((uint64_t)&kinf),KALIGN((uint64_t)&kinf),true,false);
    kinf.magic = BOOTINFO_MAGIC;
    sprint(L"Kinf at 0x%lx\n",(uint64_t)&kinf);
    //Finalize the mmap
    if(initMMAP() != EFI_SUCCESS){goto failure;}
    //Prepare for the jump
    sprint(L"Preparing to jump to 0x%lx\n",entry);
    UINTN key;
    UINTN trash;
    if(getMmap(&trash,&key,&trash,(UINT32*)&trash) == NULL){sprint(L"Failed to get mmap key\n"); goto failure;}
    status = SystemTable->BS->ExitBootServices(ImageHandle,key);
    if(status != EFI_SUCCESS){return !EFI_SUCCESS;}
    kinf.magic = BOOTINFO_MAGIC;
    uint64_t ptr = (uint64_t)&kinf;
    //Jump to the kernel
    setCR3();
    kernel((struct bootInfo*)ptr);
    for(;;){}
    failure:
    sprint(L"Press any key to return to UEFI\n");
    for(;;){} //TODO: Fix this broken pause code
    UINTN keyIn = 0;
    SystemTable->BS->WaitForEvent(1,SystemTable->ConIn->WaitForKey,&keyIn);
    return !EFI_SUCCESS;
}