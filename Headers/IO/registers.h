#pragma once

#include "types.h"

typedef struct registers{
    uint64 r15, r14, r13, r12, r11;
    uint64 r10, r9, r8, rbp, rdi;
    uint64 rsi, rdx, rcx, rbx, rax;
    uint64 rip, cs, rflags, rsp, ss, ec;
}__attribute__((packed)) regs;

extern void dumpRegisters(regs r);