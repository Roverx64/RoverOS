#include <sefi.h>
#include "bootinfo.h"
#include "bootMem.h"

extern struct bootInfo kinf;

/*!
Intilizes the montior to the best resolution possible if the GOP is present
*/
EFI_STATUS initGOP(){
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_STATUS status = locateProtocol(&gopGuid,NULL,(void**)&gop);
    if(status != EFI_SUCCESS){return status;}
    uint32_t bestMode = 0;
    uint32_t bestWidth = 0;
    uint32_t bestHeight = 0;
    UINTN buffSize = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *mode = (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION*)allocMem(buffSize,0x0,0,true,false,false);
    for(uint32_t i = 0; i < gop->Mode->MaxMode; ++i){
        gop->QueryMode(gop,i,&buffSize,&mode);
        if(mode->PixelFormat == PixelBltOnly){break;}
        if((mode->HorizontalResolution >= bestWidth) && (mode->VerticalResolution >= bestHeight)){bestMode = i;}
    }
    gop->QueryMode(gop,bestMode,&buffSize,&mode);
    kinf.ui.Xres = mode->HorizontalResolution;
    kinf.ui.Yres = mode->VerticalResolution;
    kinf.ui.pps = mode->PixelsPerScanLine;
    kinf.ui.rmask = 0xFFFFFF;
    kinf.ui.gmask = 0xFFFFFF;
    kinf.ui.bmask = 0xFFFFFF;
    kinf.ui.framebuffer = (uint64_t)gop->Mode->FrameBufferBase;
    kinf.ui.size = (uint64_t)gop->Mode->FrameBufferSize;
    freePool(mode);
    sprint(L"Set GOP mode to 0x%lxx0x%lx\n",kinf.ui.Xres,kinf.ui.Yres);
    return EFI_SUCCESS;
}