#pragma once
#include "bootinfo.h"
#include "boottypes.h"

extern struct bootInfo kinf;
extern uint64 bootstrapKernel(EFI_HANDLE ImageHandle);
extern void initConsole();