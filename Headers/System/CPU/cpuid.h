#pragma once

#include <stdint.h>

#define CPUID_FUNCTION_TSC 0x04
#define CPUID_FEATURE_TSC_INVARIANT 0x1B

#define CPUID_X2APIC_PRESENT 0x200000

static inline void CPUID(uint64_t function,uint64_t extended,uint32_t *eax,uint32_t *ebx,uint32_t *edx,uint32_t *ecx){
        uint64_t eaxRet;
        uint64_t ebxRet;
        uint64_t edxRet;
        uint64_t ecxRet;
        asm volatile("movq %4, %%rax;movq %5, %%rcx;cpuid;movq %%rax, %0;movq %%rbx, %1;movq %%rdx, %2;movq %%rcx, %3":"=r"(eaxRet),"=r"(ebxRet),"=r"(edxRet),"=r"(ecxRet):"r"((uint64_t)function), "r"((uint64_t)extended):"rax","rdx","rbx","rcx");
        *eax = (uint32_t)eaxRet;
        *ebx = (uint32_t)ebxRet;
        *edx = (uint32_t)edxRet;
        *ecx = (uint32_t)ecxRet;
}