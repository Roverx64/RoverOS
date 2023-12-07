#pragma once

#include <stdint.h>

typedef struct{
    uint64 r15, r14, r13, r12, r11;
    uint64 r10, r9, r8, rbp, rdi;
    uint64 rsi, rdx, rcx, rbx, rax;
    uint64 ec, rip, cs, rflags, rsp, ss;
}__attribute__((packed)) registers;
