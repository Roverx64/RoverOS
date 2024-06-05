#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <stdbool.h>
#include "boot.h"
#include "bootMem.h"

struct bootInfo kinf;
typedef (*kfunc)(struct bootInfo*);
extern uint64_t checkHardware(void);
extern EFI_STATUS initGOP(void);
extern EFI_STATUS initMMAP(void);
extern void mapRecords(void);

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
    Print(L"BOOT  : Polarboot loaded\n");
    //Set resolution
    if(initGOP() != EFI_SUCCESS){goto failure;}
    //Enable hardware features
    uint64_t hdw = checkHardware();
    if(!(hdw&0x1)){Print(L"BOOT  : XD is not present\n"); goto failure;}
    Print(L"BOOT  : XD is enabled\n");
    //Intilize mmap, alloc functions, and paging structs
    initMem();
    uint64_t entry = bootstrapKernel(ImageHandle);
    if(entry == 0x0){Print(L"BOOT  : Failed to bootstrap kernel\n"); goto failure;}
    kfunc kernel = (kfunc)entry;
    //Get XSDT
    if(EFI_ERROR(LibGetSystemConfigurationTable(&AcpiTableGuid,(void**)&kinf.xsdtptr))){Print(L"BOOT  : Failed to get XSDT\n"); goto failure;}
    Print(L"BOOT  : XSDT at 0x%lx\n",(uint64_t)kinf.xsdtptr);
    //Map the stack
    uint64_t stack = 0x0;
    asm volatile("movq %%rbp, %0" : "=r"(stack));
    mapPage(stack,stack,true,false);
    Print(L"BOOT  : Mapped the stack at 0x%lx\n",stack);
    //Map kinf
    mapPage(KALIGN((uint64_t)&kinf),KALIGN((uint64_t)&kinf),true,false);
    kinf.magic = BOOTINFO_MAGIC;
    Print(L"BOOT  : Kinf at 0x%lx\n",(uint64_t)&kinf);
    //Consolidate mmap
    if(initMMAP() != EFI_SUCCESS){goto failure;}
    //Prepare to jump
    Print(L"BOOT  : Preparing to jump to 0x%lx\n",entry);
    UINTN key;
    UINTN trash;
    if(LibMemoryMap(&trash,&key,&trash,(UINT32*)&trash) == NULL){Print(L"BOOT  : Failed to get mmap key\n"); goto failure;}
    EFI_STATUS status = uefi_call_wrapper(SystemTable->BootServices->ExitBootServices,2,ImageHandle,key);
    if(EFI_ERROR(status)){return !EFI_SUCCESS;}
    //Swap pml and jump
    setCR3();
    kernel(&kinf);
    for(;;){}
    failure:
    Print(L"Press any key to return to UEFI\n");
    Pause();
    return !EFI_SUCCESS;
}