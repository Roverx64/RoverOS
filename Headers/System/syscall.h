#pragma once
#include <stdint.h>

typedef int (*syscallHandler)(uint64_t);

extern void setSyscallEntry(uint64_t);
extern void enableSyscall(void);
extern void registerSyscall(syscallHandler handle, uint64_t n);