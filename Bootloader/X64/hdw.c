#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "bootPaging.h"
#include "mmap.h"
#include "bootMmap.h"

extern uint64 checkHardware(void);

EFI_STATUS initHardware(){
    Print(L"HDW   : Probing hardware\n");
    uint64 r = checkHardware();
    if(r&0x1){Print(L"HDW   : XD Present and enabled\n");}else{Print(L"XD is not present\n"); return !EFI_SUCCESS;}
    return EFI_SUCCESS;
}