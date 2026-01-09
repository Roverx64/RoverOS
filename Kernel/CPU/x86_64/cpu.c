#include <stdint.h>
#include <string.h>
#include <kprint.h>
#include <cpu.h>
#include <kmalloc.h>
#include <kernel.h>
#include <cpuid.h>

//GDT
extern void loadGDT(void *ptr);

void writeGDTSegment(uint64_t *entries, uint8_t entryn, uint64_t dpl, bool code){
    uint64_t flags = GDT_PRESENT|GDT_LM|GDT_SYSTEM|GDT_DPL(dpl);
    if(code){flags |= GDT_EXECUTE;}
    entries[entryn] = flags;
}

void writeGDTTSS(uint64_t *entries, uint8_t entryn, uint64_t base, uint64_t dpl){
    uint64_t lowerTSS = GDT_TYPE_TSS|GDT_PRESENT|GDT_DPL(dpl)|sizeof(struct TSS)|GDT_TSS_BASE0(base)|GDT_TSS_BASE1(base);
    uint64_t upperTSS = GDT_TSS_BASE2(base);
    entries[entryn] = lowerTSS;
    entries[entryn+1] = upperTSS;
}

/*!
    Only called by the APs from coreboot.
    GDTs must only be intilized once.
*/
void initGDT(uint64_t stack){
    struct gdtPointer *ptr = (struct gdtPointer*)kmalloc(sizeof(struct gdtPointer));
    uint64_t *base = (uint64_t*)kmallocAligned(sizeof(uint64_t)*8);
    ptr->limit = (sizeof(uint64_t)*8)-1;
    ptr->base = (uint64_t)base;
    memset(base,0x0,sizeof(uint64_t)*8);
    //KCode
    writeGDTSegment(base,1,0,true);
    //KData
    writeGDTSegment(base,2,0,false);
    //UCode
    writeGDTSegment(base,3,3,true);
    //UData
    writeGDTSegment(base,4,3,false);
    loadGDT(ptr);
    kinfo("Loaded GDT\n");
    //initTSS(stack,base);
}

struct gdtPointer bspgptr;
uint64_t gdtEntries[8];

//Only called by the BSP
void initBSPGDT(){
    bspgptr.limit = (sizeof(uint64_t)*8)-1;
    bspgptr.base = (uint64_t)&gdtEntries;
    memset(&gdtEntries,0x0,sizeof(uint64_t)*8);
    //KCode
    writeGDTSegment(&gdtEntries,1,0,true);
    //KData
    writeGDTSegment(&gdtEntries,2,0,false);
    //UCode
    writeGDTSegment(&gdtEntries,3,3,true);
    //UData
    writeGDTSegment(&gdtEntries,4,3,false);
    loadGDT(&bspgptr);
    kinfo("Loaded GDT\n");
}

//TSS
extern void initSyscall(void*);
extern void testSyscall(void);
extern void syscallEntryPrologue();

void switchUserTSS(void *gdt, void *tss){
    writeGDTTSS((uint64_t*)gdt,6,(uint64_t)tss,3);
}

struct TSS *initTSS(uint64_t stack, void *gdt){
    kinfo("Initilizing kernel TSS\n");
    struct TSS *tss = (struct TSS*)kmalloc(sizeof(struct TSS));
    memset(tss,0x0,sizeof(struct TSS));
    tss->RSP0 = stack;
    tss->iopb = sizeof(struct TSS);
    //Write TSS
    kinfo("Writing to GDT\n");
    writeGDTTSS(gdt,5,(uint64_t)tss,0);
    kinfo("Enabling syscalls\n");
    initSyscall(syscallEntryPrologue);
    kinfo("Wrote syscall entry to 0x%lx\n",syscallEntryPrologue);
    //Load TSS
    loadTSS(0x28);
    kinfo("Loaded TR register\n");
}

//IDT

struct idtPointer idtp;
struct idtEntry idtEntries[256];

static void writeIDT(struct idtEntry *entries, uint32_t n, uint16_t selector, void *isr){
    entries[n].type = 0xE;
    entries[n].selector = selector;
    entries[n].baseLower = IDT_BASE_LOWER((uint64_t)isr);
    entries[n].baseUpper = IDT_BASE_UPPER((uint64_t)isr);
    entries[n].present = true;
}

void overrideIDTEntry(uint32_t vector, void *handle){
    writeIDT(&idtEntries,vector,0x8,handle);
}

extern void initFaultHandlers(void);
extern void loadIDT(void*);

void initIDT(){
    asm volatile("cli");
    idtp.limit = (sizeof(struct idtEntry)*256)-1;
    idtp.base = (uint64_t)&idtEntries;
    kinfo("[IDTP|Limit: 0x%llx|Base: 0x%llx]\n",(uint64_t)idtp.limit,(uint64_t)idtp.base);
    memset(&idtEntries,0x0,sizeof(struct idtEntry)*256);
    writeIDT(&idtEntries,0,0x8,(void*)isr0);
    writeIDT(&idtEntries,1,0x8,(void*)isr1);
    writeIDT(&idtEntries,2,0x8,(void*)isr2);
    writeIDT(&idtEntries,3,0x8,(void*)isr3);
    writeIDT(&idtEntries,4,0x8,(void*)isr4);
    writeIDT(&idtEntries,5,0x8,(void*)isr5);
    writeIDT(&idtEntries,6,0x8,(void*)isr6);
    writeIDT(&idtEntries,7,0x8,(void*)isr7);
    writeIDT(&idtEntries,8,0x8,(void*)isr8);
    writeIDT(&idtEntries,9,0x8,(void*)isr9);
    writeIDT(&idtEntries,10,0x8,(void*)isr10);
    writeIDT(&idtEntries,11,0x8,(void*)isr11);
    writeIDT(&idtEntries,12,0x8,(void*)isr12);
    writeIDT(&idtEntries,13,0x8,(void*)isr13);
    writeIDT(&idtEntries,14,0x8,(void*)isr14);
    writeIDT(&idtEntries,15,0x8,(void*)isr15);
    writeIDT(&idtEntries,16,0x8,(void*)isr16);
    writeIDT(&idtEntries,17,0x8,(void*)isr17);
    writeIDT(&idtEntries,18,0x8,(void*)isr18);
    writeIDT(&idtEntries,19,0x8,(void*)isr19);
    writeIDT(&idtEntries,20,0x8,(void*)isr20);
    writeIDT(&idtEntries,21,0x8,(void*)isr21);
    writeIDT(&idtEntries,22,0x8,(void*)isr22);
    writeIDT(&idtEntries,23,0x8,(void*)isr23);
    writeIDT(&idtEntries,24,0x8,(void*)isr24);
    writeIDT(&idtEntries,25,0x8,(void*)isr25);
    writeIDT(&idtEntries,26,0x8,(void*)isr26);
    writeIDT(&idtEntries,27,0x8,(void*)isr27);
    writeIDT(&idtEntries,28,0x8,(void*)isr28);
    writeIDT(&idtEntries,29,0x8,(void*)isr29);
    writeIDT(&idtEntries,30,0x8,(void*)isr30);
    writeIDT(&idtEntries,31,0x8,(void*)isr31);
    writeIDT(&idtEntries,32,0x8,(void*)irq0);
    writeIDT(&idtEntries,33,0x8,(void*)irq1);
    writeIDT(&idtEntries,34,0x8,(void*)irq2);
    writeIDT(&idtEntries,35,0x8,(void*)irq3);
    writeIDT(&idtEntries,36,0x8,(void*)irq4);
    writeIDT(&idtEntries,37,0x8,(void*)irq5);
    writeIDT(&idtEntries,38,0x8,(void*)irq6);
    writeIDT(&idtEntries,39,0x8,(void*)irq7);
    writeIDT(&idtEntries,40,0x8,(void*)irq8);
    writeIDT(&idtEntries,41,0x8,(void*)irq9);
    writeIDT(&idtEntries,42,0x8,(void*)irq10);
    writeIDT(&idtEntries,43,0x8,(void*)irq11);
    writeIDT(&idtEntries,44,0x8,(void*)irq12);
    writeIDT(&idtEntries,45,0x8,(void*)irq13);
    writeIDT(&idtEntries,46,0x8,(void*)irq14);
    writeIDT(&idtEntries,47,0x8,(void*)irq15);
    kinfo("Set entries\n");
    loadIDT((void*)&idtp);
    kinfo("Set IDT\n");
    asm volatile("sti");
}

void bootstrapCPU(){
    initBSPGDT();
    initIDT();
}

extern intHandler generalProtectionFault(registers *reg);
extern intHandler divByZero(registers *reg);
extern intHandler debugException(registers *reg);
extern intHandler pageFault(registers *reg);
extern intHandler invalidOpcode(registers *reg);

intHandler handlers[30];
extern intHandler pageFault(registers *reg);

void isr(registers reg, uint32_t ec){
    //Built-in handlers
    switch(ec){
        case 0x0:
        divByZero(&reg);
        break;
        case 0x1:
        debugException(&reg);
        break;
        case 0x6:
        invalidOpcode(&reg);
        break;
        case 0x8:
        kpanic("Double fault\n",reg.rip);
        break;
        case 0xD:
        generalProtectionFault(&reg);
        break;
        case 0xE:
        pageFault(&reg);
        break;
        default:
        break;
    }
    if(handlers[ec] != NULL){
        intHandler fnc = (intHandler*)handlers[ec];
        fnc(&reg);
        return;
    }
    kerror("Unhandled ISR 0x%x\n",ec);
    for(;;){asm("hlt");}
}

void irq(registers reg, uint64_t irqn){
    if(irqn < 32){kerror("Invalid IRQ of 0x%x\n",irqn); return;}
    if(handlers[irqn] == NULL){kwarn("Unhandled IRQ\n"); return;}
    intHandler fnc = (intHandler*)handlers[irqn];
    fnc(&reg);
    return;
}

void addInterruptHandler(uint32_t vector, intHandler handler){
    kinfo("Adding handler for vector 0x%x\n",vector);
    handlers[vector] = handler;
}

void removeInterruptHandler(uint32_t vector){
    handlers[vector] = NULL;
}

//Returns 0 if it failed to allocate one
uint16_t allocateInterrupt(intHandler handler){
    for(uint32_t i = 19; i < 255; ++i){
        if(handlers[i] != NULL){continue;}
        handlers[i] = handler;
        return i;
    }
    return 0;
}

//TSC stuff

void initTSC(){
    //Check for TSC
    uint32_t check = 0x0;
    uint32_t trash;
    CPUID(0x1,&trash,&check,&trash);
    if(!(check&CPU_TSC_PRESENT)){kinfo("No TSC detected\n"); return;}
    //Check for invariance
    //CPUID(0x80000007,&trash,);
}
