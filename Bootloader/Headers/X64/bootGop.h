#pragma once
#include <efi.h>

#define GUI_ENABLE_CAP 1 //Comment the out to disable it
#define GUI_MAX_XRES 1920
#define GUI_MAX_YRES 1080

extern EFI_STATUS findBestMode();