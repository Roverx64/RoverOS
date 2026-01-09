#include <sefi.h>
#include <stdint.h>
#include <stdbool.h>
#include <polarboot.h>
#include <boot.h>
#include <bootMem.h>
#include <bootElf.h>
#include <bootFile.h>

EFIFile fl;

uint64_t bootstrapKernel(EFI_HANDLE handle){
    clearMem(&fl,sizeof(fl));
    //Load kernel
    if(openVolume(&fl,handle) != EFI_SUCCESS){sprint(L"Failed to open volume\n"); return 0x0;}
    if(openFile(&fl,L"Fortuna.bin",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY) != EFI_SUCCESS){sprint(L"Failed to open Fortuna.bin\n"); return 0x0;}
    if(getFileInfo(&fl) != EFI_SUCCESS){sprint(L"Failed to get file info\n"); return 0x0;}
    sprint(L"Loading kernel\n");
    uint64_t entry = loadELF(&fl,true);
    if(entry == 0x0){return 0x0;}
    sprint(L"Got entry at 0x%lx\n",entry);
    closeFile(&fl);
    //Load ramdisk
    if(openFile(&fl,L"init.rd",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY) != EFI_SUCCESS){sprint(L"No ramdisk found\n"); goto exit;}
    if(getFileInfo(&fl) != EFI_SUCCESS){sprint(L"Failed to get file info\n"); goto exit;}
    void *buff = allocMem(fl.Info->FileSize+PAGE_SZ,0x0,MMAP_TYPE_OTHER,false,true,false);
    if(readFile(&fl,buff,fl.Info->FileSize) != EFI_SUCCESS){sprint(L"Failed to read file\n"); goto exit;}
    sprint(L"Loaded ramdisk to 0x%lx\n",(uint64_t)buff);
    kinf.rdptr = (uint64_t)buff;
    kinf.rdsz = fl.Info->FileSize;
    closeFile(&fl);
    //Load symbol file
    if(openFile(&fl,L"Fortuna.sym",EFI_FILE_MODE_READ,EFI_FILE_READ_ONLY) != EFI_SUCCESS){sprint(L"Failed to open Fortuna.sym\n"); return 0x0;}
    if(getFileInfo(&fl) != EFI_SUCCESS){sprint(L"Failed to get file info\n"); return 0x0;}
    buff = allocMem(fl.Info->FileSize+PAGE_SZ,0x0,MMAP_TYPE_OTHER,false,false,false);
    if(readFile(&fl,buff,fl.Info->FileSize) != EFI_SUCCESS){sprint(L"Failed to read file\n"); goto exit;}
    sprint(L"Loaded Fortuna.sym to 0x%lx\n",(uint64_t)buff);
    kinf.mem.symptr = (uint64_t)buff;
    kinf.mem.symsz = fl.Info->FileSize;
    exit:
    closeVolume(&fl);
    return entry;
}