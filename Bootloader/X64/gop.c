#include <efi.h>
#include <efilib.h>
#include <stdint.h>
#include <stdbool.h>
#include "boot.h"
#include "bootMem.h"

EFI_STATUS initGOP(){
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    LibLocateProtocol(&gopGuid,(void**)&gop);
    uint32_t bestMode = 0;
    uint32_t bestWidth = 0;
    uint32_t bestHeight = 0;
    UINTN buffSize = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *mode = (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION*)allocMem(buffSize,0x0,0,true,false,false);
    for(uint32_t i = 0; i < gop->Mode->MaxMode; ++i){
        uefi_call_wrapper(gop->QueryMode,4,gop,i,&buffSize,&mode);
        if(mode->PixelFormat == PixelBltOnly){break;}
        if((mode->HorizontalResolution >= bestWidth) && (mode->VerticalResolution >= bestHeight)){bestMode = i;}
    }
    uefi_call_wrapper(gop->QueryMode,4,gop,bestMode,&buffSize,&mode);
    kinf.ui.Xres = mode->HorizontalResolution;
    kinf.ui.Yres = mode->VerticalResolution;
    kinf.ui.pps = mode->PixelsPerScanLine;
    kinf.ui.rmask = 0xFFFFFF;
    kinf.ui.gmask = 0xFFFFFF;
    kinf.ui.bmask = 0xFFFFFF;
    kinf.ui.framebuffer = (void*)gop->Mode->FrameBufferBase;
    kinf.ui.sz = (uint64_t)gop->Mode->FrameBufferSize;
    FreePool(mode);
    Print(L"GOP   : Set mode to 0x%lxx0x%lx\n",kinf.ui.Xres,kinf.ui.Yres);
    return EFI_SUCCESS;
}