#include <efi.h>
#include <efilib.h>
#include "boottypes.h"
#include "bootMmap.h"
#include "bootFile.h"
#include "boot.h"
#include "bootPaging.h"

//For running commands to debug and test the OS

uint64 cmdtoi(uint16 *str){
    uint64 v = 0;
    for(int i = 0; str[i] != '\0'; ++i){
        v += (uint64)str[i];
        if(i%2 == 0){v += str[i]^str[i];}
    }
    return v;
}

uint16 *commands[30];
int ccurs = 0;
void registerCommand(uint16 *command){
    commands[ccurs] = command;
    ++ccurs;
}

bool ret = false;

void proccessCommand(uint16 *str){
    switch(cmdtoi(str)){
        case 0x1A9: //help
        Print(L"%s",commands[0]);
        for(int i = 1; i < ccurs; ++i){
            if(i%5 == 0){Print(L"\n");}else{Print(L", ");}
            Print(L"%s",commands[i]);
        }
        Print(L"\n");
        break;
        case 0x207: //clear
        uefi_call_wrapper(ST->ConOut->ClearScreen,1,ST->ConOut);
        break;
        case 0x28B: //reboot
        uefi_call_wrapper(ST->RuntimeServices->ResetSystem,4,EfiResetCold,EFI_SUCCESS,12,"Command used");
        break;
        case 0x2E4: //dumpPMAP
        dumpPMAP(false);
        break;
        case 0x417: //dumpEFI_PMAP
        dumpPMAP(true);
        break;
        case 0x365: //continue
        ret = true;
        break;
        case 0x214: //setCR3
        setCR3();
        Print(L"Set the CR3\n");
        break;
        case 0x269: //mapMMAP
        mapMMAP();
        break;
        case 0x2DE: //testMap
        Print(L"run 'dumpPMAP' to verify changes\n");
        mapPage(PAGE_SZ*800,0x0,false,false,true);
        Print(L"Mapped 0x%llx->0x%llx\n",PAGE_SZ*800,0x0);
        break;
        case 0x27E: //dumpCR3
        uint64 cr3 = 0x0;
        asm volatile("movq %%cr3, %0" : "=r"(cr3));
        Print(L"CR3: 0x%llx\n",cr3);
        cr3 = 0x0;
        break;
        default:
        Print(L"Unknown cmd 0x%llx\n",cmdtoi(str));
        break;
    }
}

void initConsole(){
    //Register commands for help
    registerCommand(L"help");
    registerCommand(L"clear");
    registerCommand(L"reboot");
    registerCommand(L"testMap");
    registerCommand(L"dumpPMAP");
    registerCommand(L"mapMMAP");
    registerCommand(L"setCR3");
    registerCommand(L"dumpEFI_PMAP");
    registerCommand(L"continue");
    uint16 *inp = AllocatePool(30);
    for(;ret == false;){
        Input(L">",inp,30);
        Print(L"\n");
        proccessCommand(inp);
        SetMem(inp,30,0x0);
    }
    FreePool(inp);
    ret = false;
}