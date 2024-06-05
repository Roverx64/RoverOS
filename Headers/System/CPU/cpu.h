#pragma once

#include "cpuid.h"
#include <stdint.h>

//CPU related funtions

//MSR

static inline void wrmsr(uint64_t msr, uint64_t val){
    asm volatile("movq %0, %%rcx;movq %2, %%rax;movq %1, %%rdx;wrmsr"::"r"(msr),"r"((uint64_t)(val>>32)),"r"((uint64_t)(val&0xFFFFFFFF)):"rcx","rdx","rax");
}

static inline uint64_t rdmsr(uint64_t msr){
    uint64_t low;
    uint64_t high;
    asm volatile("movq %2, %%rcx;rdmsr;movq %%rax, %0;movq %%rdx, %1":"=r"(low),"=r"(high):"r"(msr):"rcx","rdx","rax");
    return (uint64_t)low|((uint64_t)high<<32);
}

//MMIO

static inline uint64_t rlmmio(uint64_t bar, uint64_t reg){
    uint64_t data;
    asm volatile("add %1, %2; movq %2, %%rdx; movq (%%rdx), %0":"=r" (data):"r" (bar), "r" (reg):"rdx");
    return data;
}

static inline uint32_t rdmmio(uint64_t bar, uint64_t reg){
    uint64_t data = 0;
    asm volatile("add %1, %2; movq %2, %%rdx; movq (%%rdx), %0":"=r" (data):"r" (bar), "r" (reg):"rdx");
    return data&0xFFFFFFFF;
}

static inline void wlmmio(uint64_t bar, uint64_t reg, uint64_t data){
    asm volatile("add %1, %2;movq %2, %%rdx; movq %0, (%%rdx)"::"r" (data), "r" (bar), "r"(reg):"rdx");
}

static inline void wdmmio(uint64_t bar, uint64_t reg, uint32_t data){
    asm volatile("add %1, %2;movq %2, %%rdx; mov %0, (%%rdx)"::"r" (data), "r" (bar), "r"(reg):"rdx");
}

typedef struct{
    uint64_t r15, r14, r13, r12, r11;
    uint64_t r10, r9, r8, rbp, rdi;
    uint64_t rsi, rdx, rcx, rbx, rax;
    uint64_t ec, rip, cs, rflags, rsp, ss;
}__attribute__((packed)) registers;

//CPU structures

//GDT
struct gdtPointer{
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));

#define GDT_RW (uint64_t)1<<43
#define GDT_CONFORMING (uint64_t)1<<44
#define GDT_EXECUTE (uint64_t)1<<43
#define GDT_SYSTEM (uint64_t)1<<44
#define GDT_DPL(dpl) ((uint64_t)dpl<<45)
#define GDT_PRESENT (uint64_t)1<<47
#define GDT_LM (uint64_t)1<<53
#define GDT_TYPE_TSS (uint64_t)0x9<<40
#define GDT_TSS_BASE0(addr) ((addr&0xFFFFF)<<16)
#define GDT_TSS_BASE1(addr) (((addr>>24)&0xFF)<<48)
#define GDT_TSS_BASE2(addr) ((addr>>32)&0xFFF)
extern void initGDT();
extern void writeGDTTSS(uint64_t *entries, uint8_t entryn, uint64_t base, uint64_t dpl);

//TSS
struct TSS{
    uint32_t res0;
    uint64_t RSP0;
    uint64_t RSP1;
    uint64_t RSP2;
    uint64_t res1;
    uint64_t IST0;
    uint64_t IST1;
    uint64_t IST2;
    uint64_t IST3;
    uint64_t IST4;
    uint64_t IST5;
    uint64_t IST6;
    uint64_t IST7;
    uint64_t res2;
    uint32_t res3;
    uint32_t iopb;
}__attribute__((packed));

extern void loadTSS(uint8_t segment);

//IDT
struct idtPointer{
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));

struct idtEntry{
    uint16_t baseLower : 16; //Lower bytes of isr
    uint16_t selector : 16;
    uint8_t ist : 3; //Interrupt stack table
    uint8_t ign0 : 5;
    uint8_t type : 4;
    uint8_t a0 : 1;
    uint8_t dpl : 2; //Privilege level
    uint8_t present : 1;
    uint64_t baseUpper : 48; //Upper bytes of isr
    uint32_t reserved : 32;
}__attribute__((packed));

#define IDT

#define IDT_BASE_LOWER(addr) (addr&0xFFFF)
#define IDT_BASE_UPPER(addr) (addr>>16)

extern void initIDT();
extern void overrideIDTEntry(uint32_t vector, void *handle);

typedef void (*intHandler)(registers);
extern void addInterruptHandler(uint32_t vector, intHandler handler);

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();
