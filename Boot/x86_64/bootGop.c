#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "bootGop.h"

EFI_STATUS findBestMode(){
    uint32 xRes = 0x0;
    uint32 yRes = 0x0;
    EFI_STATUS status = EFI_SUCCESS;
    uint32 moden = 0x0;
    //Get protocol
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GUID guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    UI_INTERFACE uif;
    status = LibLocateProtocol(&guid,(void**)&gop);
    if(EFI_ERROR(status)){Print(L"Failed to locate GOP\n"); return status;}
    //Search through modes
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *inf = (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION*)AllocatePool(gop->Mode->SizeOfInfo);
    for(int i = 0; i < gop->Mode->MaxMode; ++i){
        status = uefi_call_wrapper(gop->QueryMode,4,gop,i,gop->Mode->SizeOfInfo,&inf);
        if(EFI_ERROR(status)){Print(L"Failed to query mode\n"); return status;}
        #ifdef GUI_ENABLE_CAP
        if(inf->HorizontalResolution > GUI_MAX_XRES || inf->VerticalResolution > GUI_MAX_YRES){continue;}
        #endif
        if(inf->HorizontalResolution < xRes || inf->VerticalResolution < yRes){continue;}
        if(inf->PixelFormat == PixelBltOnly){continue;}
        //Found best mode
        xRes = inf->HorizontalResolution;
        yRes = inf->VerticalResolution;
        moden = i;
    }
    //Set mode
    status = uefi_call_wrapper(gop->SetMode,2,gop,moden);
    if(EFI_ERROR(status)){Print(L"Failed to set mode\n"); return status;}
    status = uefi_call_wrapper(gop->QueryMode,4,gop,moden,gop->Mode->SizeOfInfo,&inf);
    Print(L"Set resolution to 0x%lxx0x%lx\n",inf->HorizontalResolution,inf->VerticalResolution);
    Print(L"Buffer at 0x%llx with size of 0x%llx bytes\n",gop->Mode->FrameBufferBase,gop->Mode->FrameBufferSize);
    //Write boot info
    kinf.ui.Xres = inf->HorizontalResolution;
    kinf.ui.Yres = inf->VerticalResolution;
    kinf.ui.pps = inf->PixelsPerScanLine;
    kinf.ui.rmask = inf->PixelInformation.RedMask;
    kinf.ui.gmask = inf->PixelInformation.GreenMask;
    kinf.ui.bmask = inf->PixelInformation.BlueMask;
    kinf.ui.resmask = inf->PixelInformation.ReservedMask;
    kinf.ui.sz = gop->Mode->FrameBufferSize;
    kinf.ui.framebuffer = (void*)gop->Mode->FrameBufferBase;
    FreePool(inf);
    return EFI_SUCCESS;
}