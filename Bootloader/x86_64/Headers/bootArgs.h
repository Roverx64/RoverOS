#pragma once
#include <sefi.h>
#include <stdint.h>
#include "bootFile.h"

extern uint64_t getArgv(uint64_t arg);
extern void initArgs(EFI_HANDLE ImageHandle);