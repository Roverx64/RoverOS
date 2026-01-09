#include <stdint.h>
#include <kprint.h>
#include <fdebug.h>
#include <paging.h>
#include <cpu.h>
#include <ksym.h>

void stackTrace(uint64_t rbp){
    struct stackFrame *frame = (struct stackFrame*)rbp;
    knone("#=========Stack==trace=========#\n");
    //Unwind stack
    while(frame != NULL){
        ELF64Sym *sym = getSymbol(frame->ret);
        if(sym){
            char *nm = symbolToName(sym);
            knone("%s()->",nm);
        }
        else{
            knone("unknown()->");
        }
        uint64_t stack = ((frame->framePointer-(uint64_t)frame)/8)-2;
        if(frame->framePointer == 0x0){stack = 0x0;}
        knone("[RIP:0x%lx][0x%lx items in frame]\n",frame->ret,stack);
        //Calc difference and dump stack
        if(frame->framePointer == 0x0){break;}
        knone("\u2193\n");
        /*uint64_t *stk = (uint64_t*)((uint64_t)frame+sizeof(frame)+sizeof(uint64_t));
        for(uint64_t z = 0; z < stack; ++z){
            knone("    [RSP 0x%lx]->[0x%lx]\n",z,stk[z]);
        }*/
        frame = (struct stackFrame*)(frame->framePointer);
    }
}

void dumpRegisters(registers *reg){
    uint64_t cr2 = 0x0;
    uint64_t cr3 = 0x0;
    asm volatile("movq %%cr3, %0" : "=r"(cr3));
    asm volatile("movq %%cr2, %0" : "=r"(cr2));
    knone("#===========Registers==========#\n");
    knone("RAX: 0x%lx RCX: 0x%lx RBX: 0x%lx\n",reg->rax,reg->rcx,reg->rbx);
    knone("RDI: 0x%lx RSI: 0x%lx RDX: 0x%lx\n",reg->rdi,reg->rsi,reg->rdx);
    knone("RIP: 0x%lx RBP: 0x%lx RSP: 0x%lx\n",reg->rip,reg->rbp,reg->rsp);
    knone("R08: 0x%lx R09: 0x%lx R10: 0x%lx\n",reg->r8,reg->r9,reg->r10);
    knone("R11: 0x%lx R12: 0x%lx R13: 0x%lx\n",reg->r11,reg->r12,reg->r13);
    knone("R14: 0x%lx R15: 0x%lx\n",reg->r14,reg->r15);
    knone("CS: 0x%lx SS: 0x%lx EC: 0x%lx\n",reg->cs,reg->ss,reg->ec);
    knone("RFLAGS: 0x%lx CR2: 0x%lx CR3: 0x%lx\n",reg->rflags,cr2,cr3);
    stackTrace(reg->rbp);
    knone("#==============================#\n");
}

void setCPUBreakpoint(uint64_t addr){

}