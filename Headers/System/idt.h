#pragma once
#include <stdint.h>

struct idtPointer{
    uint16 limit;
    uint64 base;
}__attribute__((packed));

struct idtEntry{
    uint16 baseLower : 16; //Lower bytes of isr
    uint16 selector : 16;
    uint8 ist : 3; //Interrupt stack table
    uint8 ign0 : 5;
    uint8 type : 4;
    uint8 a0 : 1;
    uint8 dpl : 2; //Privilege level
    uint8 present : 1;
    uint64 baseUpper : 48; //Upper bytes of isr
    uint32 reserved : 32;
}__attribute__((packed));

#define IDT_BASE_LOWER(addr) (addr&0xFFFF)
#define IDT_BASE_UPPER(addr) (addr>>16)

extern void loadIDT(void *ptr);
extern void initIDT();

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