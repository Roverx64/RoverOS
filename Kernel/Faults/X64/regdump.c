#include <stdint.h>
#include <debug.h>
#include <registers.h>
#include "interrupt.h"

inline void dumpRegisters(registers r){
    kdebug(DNONE,"RAX: 0x%lx RCX: 0x%lx RBX: 0x%lx\n",r.rax,r.rcx,r.rbx);
    kdebug(DNONE,"RDI: 0x%lx RSI: 0x%lx RDX: 0x%lx\n",r.rdi,r.rsi,r.rdx);
    kdebug(DNONE,"RIP: 0x%lx RBP: 0x%lx RSP: 0x%lx\n",r.rip,r.rbp,r.rsp);
    kdebug(DNONE,"R08: 0x%lx R09: 0x%lx R10: 0x%lx\n",r.r8,r.r9,r.r10);
    kdebug(DNONE,"R11: 0x%lx R12: 0x%lx R13: 0x%lx\n",r.r11,r.r12,r.r13);
    kdebug(DNONE,"R14: 0x%lx R15: 0x%lx\n",r.r14,r.r15);
    kdebug(DNONE,"CS: 0x%lx SS: 0x%lx EC: 0x%lx\n",r.cs,r.ss,r.ec);
    kdebug(DNONE,"RFLAGS: 0x%lx\n",r.rflags);
}