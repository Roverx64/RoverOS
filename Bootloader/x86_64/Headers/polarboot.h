#pragma once

#include <sefi.h>
#include <boot.h>

extern EFI_STATUS initGOP();
extern void initMem();
typedef void (__attribute__((sysv_abi)) *kfunc)(struct bootInfo*);
extern uint64_t bootstrapKernel(EFI_HANDLE handle);
extern uint64_t checkHardware(void); //!< env.asm