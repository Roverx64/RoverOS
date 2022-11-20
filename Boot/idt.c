#include <efi.h>
#include <efilib.h>
#include "idt.h"
#include "types.h"
#include "registers.h"
#include "functions.h"
#include "interrupt.h"
#include "boot.h"

extern void fidt(void *idt);

extern idtPtr idtp;

idtEntry idt[256] __attribute__((aligned(4096)));;
idtPtr idtp __attribute__((aligned(4096)));;

void tick(){
    return;
}

void writeIdt(uint32 n, uint64 isr){
    idt[n].low = (uint16)(isr&0xFFFF);
    idt[n].mid = (uint16)((isr>>16)&0xFFFF);
    idt[n].high = (uint32)(isr>>32);
    idt[n].ist = 0x0;
    idt[n].flags = 0x8E;
    idt[n].segment = 0x8;
    idt[n].zero = 0;
}

void initIDT(EFI_HANDLE ImageHandle, struct bootInfoS *bi){
    asm("cli");
    Print(L"Exiting boot services\n");
    uefi_call_wrapper(ST->BootServices->ExitBootServices,2,ImageHandle,bi->memory.mmapKey);
    Print(L"Loading IDT\n");
    idtp.limit = sizeof(idtEntry)*256-1;
    idtp.base = (uint64)&idt;
    //SetMem((uint8*)&idt, sizeof(idt)*256, 0x0);
    Print(L"Writing entries\n");
    writeIdt(0, (uint64)isr0);
    writeIdt(1, (uint64)isr1);
    writeIdt(2, (uint64)isr2);
    writeIdt(3, (uint64)isr3);
    writeIdt(4, (uint64)isr4);
    writeIdt(5, (uint64)isr5);
    writeIdt(6, (uint64)isr6);
    writeIdt(7, (uint64)isr7);
    writeIdt(8, (uint64)isr8);
    writeIdt(9, (uint64)isr9);
    writeIdt(10, (uint64)isr10);
    writeIdt(11, (uint64)isr11);
    writeIdt(12, (uint64)isr12);
    writeIdt(13, (uint64)isr13);
    writeIdt(14, (uint64)isr14);
    writeIdt(15, (uint64)isr15);
    writeIdt(16, (uint64)isr16);
    writeIdt(17, (uint64)isr17);
    writeIdt(18, (uint64)isr18);
    writeIdt(19, (uint64)isr19);
    writeIdt(20, (uint64)isr20);
    writeIdt(21, (uint64)isr21);
    writeIdt(22, (uint64)isr22);
    writeIdt(23, (uint64)isr23);
    writeIdt(24, (uint64)isr24);
    writeIdt(25, (uint64)isr25);
    writeIdt(26, (uint64)isr26);
    writeIdt(27, (uint64)isr27);
    writeIdt(28, (uint64)isr28);
    writeIdt(29, (uint64)isr29);
    writeIdt(30, (uint64)isr30);
    writeIdt(31, (uint64)isr31);
    writeIdt(32, (uint64)irq0);
    writeIdt(33, (uint64)irq1);
    writeIdt(34, (uint64)irq2);
    writeIdt(35, (uint64)irq3);
    writeIdt(36, (uint64)irq4);
    writeIdt(37, (uint64)irq5);
    writeIdt(38, (uint64)irq6);
    writeIdt(39, (uint64)irq7);
    writeIdt(40, (uint64)irq8);
    writeIdt(41, (uint64)irq9);
    writeIdt(42, (uint64)irq10);
    writeIdt(43, (uint64)irq11);
    writeIdt(44, (uint64)irq12);
    writeIdt(45, (uint64)irq13);
    writeIdt(46, (uint64)irq14);
    writeIdt(47, (uint64)irq15);
    Print(L"Setting pointer\n");
    fidt((void*)&idtp);
    Print(L"Loaded IDT\n");
    bi->io.idtp = (uintptr)&idtp;
    bi->io.idt = (uintptr)&idt;
    return;
}

irh handlers[256];
uint32 errcode;

uint32 getErrCode(){
    return errcode;
}

void isrHandler(regs r, uint32 ec){
    if(handlers[ec] != NULL){
        errcode = ec;
        irh handler = handlers[ec];
        handler(r); 
    }
    return;
}

void irqHandler(int interrupt, regs r){
    if(interrupt >= 40){
        outp(0xA0, 0x20);
    }
    outp(0x20, 0x20);
    if(handlers[interrupt] != NULL){
        irh handler = handlers[interrupt];
        handler(r);
        return;
    }
    Print(L"Unhandled IRQ 0x%x\n",interrupt);
    return;
}

void addInterruptHandler(irh func, int n){
    handlers[n] = func;
}

void loadHandlers(){
    //Setup pit
    addInterruptHandler(tick,IRQ0);
    uint32 div = 1193180/40; //40 = Freq
    outp(0x43, 0x36);
    uint8 l = (uint8)(div&0xFF);
    uint8 h = (uint8)((div>>8)&0xFF);
    outp(0x40, l);
    outp(0x40, h);
    //Enable interrupts
    asm("sti");
    Print(L"Enabled interrupts\n");
}