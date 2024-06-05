#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <stdbool.h>
#include <boot.h>
#include <bootMem.h>
#include <bootElf.h>
#include <bootFile.h>

EFIFile fl;

uint64_t bootstrapKernel(EFI_HANDLE handle){
    ZeroMem(&fl,sizeof(fl));
    //Load kernel
    if(openVolume(&fl,handle) != EFI_SUCCESS){Print(L"KLOAD : Failed to open volume\n"); return 0x0;}
    if(openFile(&fl,L"Fortuna.bin",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY) != EFI_SUCCESS){Print(L"KLOAD : Failed to open Fortuna.bin\n"); return 0x0;}
    if(getFileInfo(&fl) != EFI_SUCCESS){Print(L"KLOAD : Failed to get file info\n"); return 0x0;}
    Print(L"KLOAD : Loading kernel\n");
    uint64_t entry = loadELF(&fl,true);
    if(entry == 0x0){return 0x0;}
    Print(L"KLOAD : Got entry at 0x%lx\n",entry);
    //Cleanup
    closeFile(&fl);
    closeVolume(&fl);
    return entry;
}