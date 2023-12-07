#pragma once
#include "bootinfo.h"
#include "boottypes.h"
#include "bootArgs.h"
#include "polarboot.h"

extern struct bootInfo kinf;
extern uint64 bootstrapKernel(EFI_HANDLE ImageHandle);
extern EFI_STATUS initGOP();
extern EFI_STATUS initPaging();
extern EFI_STATUS initMMAP();
extern void setCR3();