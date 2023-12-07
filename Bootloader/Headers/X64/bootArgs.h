#pragma once
#include <efi.h>
#include "boottypes.h"
#include "bootFile.h"

extern uint64 getArgv(uint64 arg);
extern void initArgs(EFI_HANDLE ImageHandle);