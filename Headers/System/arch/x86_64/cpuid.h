#pragma once

#include <stdint.h>

#define CPUID_FEATURE_TSC_PRESENT 1<<3 //Bit 4
#define CPUID_FEATURE_TSC_INVARIANT 0x1B

#define CPUID_X2APIC_PRESENT 0x200000

static inline void CPUID(uint64_t function,uint32_t *ebx,uint32_t *edx,uint32_t *ecx){
        uint64_t ebxRet;
        uint64_t edxRet;
        uint64_t ecxRet;
        asm volatile("movq %2, %%rax;movq %3, %%rcx;cpuid;movq %%rdx, %0;movq %%rcx, %1":"=r"(ebxRet),"=r"(edxRet),"=r"(ecxRet):"r"((uint64_t)function):"rax","rdx","rcx","rbx");
        *ebx = (uint32_t)ebxRet;
        *edx = (uint32_t)edxRet;
        *ecx = (uint32_t)ecxRet;
}