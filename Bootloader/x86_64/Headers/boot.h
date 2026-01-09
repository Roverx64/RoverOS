#pragma once
#include "bootArgs.h"
#include "bootinfo.h"
#include "polarboot.h"
#include <stdint.h>

extern struct bootInfo kinf;
extern uint64_t bootstrapKernel(EFI_HANDLE handle);
extern void setCR3();