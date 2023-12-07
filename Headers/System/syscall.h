#pragma once
#include <stdint.h>

typedef (*syscallHandler)(uint64);

extern void setSyscallEntry(uint64);
extern void enableSyscall(void);
extern void registerSyscall(syscallHandler handle, uint64 n);