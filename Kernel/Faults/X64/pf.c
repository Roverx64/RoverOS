#include <stdint.h>
#include <debug.h>
#include <registers.h>
#include "interrupt.h"
#include "paging.h"

intHandler pageFault(registers reg){
    kdebug(DNONE,"#==========Page=Fault==========#\n");
    uint64 faultAddr = 0x0;
    uint64 cr3 = 0x0;
    asm volatile("movq %%cr3, %0" : "=r"(cr3));
    asm volatile("movq %%cr2, %0" : "=r"(faultAddr));
    kdebug(DNONE,"ADDR=0x%llx\n",faultAddr);
    uint8 present = reg.ec&(1<<0);
    uint8 write = reg.ec&(1<<1);
    uint8 user = reg.ec&(1<<2);
    uint8 resv = reg.ec&(1<<3);
    uint8 fetch = reg.ec&(1<<4);
    kdebug(DNONE,"CR3=0x%llx\n",cr3);
    kdebug(DNONE,"CPL=");
    if(!user){kdebug(DNONE,"Supervisor\n");}
    else{kdebug(DNONE,"User\n");}
    uint64 page = getPageInfo(cr3,faultAddr);
    if(page == 0x0){kdebug(DNONE,"Page not found in cr3\n"); goto cause;}
    kdebug(DNONE,"RAW=0x%llx\n",page);
    kdebug(DNONE,"PHYS=0x%llx\n",TABLE_BASE(page));
    kdebug(DNONE,"RIP=0x%lx\n",reg.rip);
    kdebug(DNONE,"FLAGS=");
    if(page&PG_PRESENT){kdebug(DNONE,"P");}else{kdebug(DNONE,"-");}
    if(page&PG_WRITE){kdebug(DNONE,"W");}else{kdebug(DNONE,"R");}
    if(page&PG_USER){kdebug(DNONE,"U");}else{kdebug(DNONE,"S");}
    if(page&PG_PWT){kdebug(DNONE,"T");}else{kdebug(DNONE,"-");}
    if(page&PG_PCD){kdebug(DNONE,"C");}else{kdebug(DNONE,"-");}
    if(page&PG_ACCESSED){kdebug(DNONE,"A");}else{kdebug(DNONE,"-");}
    if(page&PG_DIRTY){kdebug(DNONE,"D");}else{kdebug(DNONE,"-");}
    if(page&PG_GLOBAL){kdebug(DNONE,"G");}else{kdebug(DNONE,"-");}
    if(page&PG_PAT){kdebug(DNONE,"O");}else{kdebug(DNONE,"-");}
    if(page&PG_NX){kdebug(DNONE,"-");}else{kdebug(DNONE,"X");}
    kdebug(DNONE,"\n");
    cause:
    //Determine cause
    kdebug(DNONE,"Cause: ");
    if(resv){
        kdebug(DNONE,"Wrote 1 to reserved bit(s)\n");
        goto end;
    }
    if(page == 0x0){
        kdebug(DNONE,"Non-existent page\n");
        goto end;
    }
    if(!present){
        if(write){kdebug(DNONE, "Wrote to ");}
        else{kdebug(DNONE,"Read from ");}
        kdebug(DNONE,"unmapped page\n");
        goto end;
    }
    if(present && write && !(page&PG_WRITE)){
        kdebug(DNONE,"Wrote to read-only page\n");
        goto end;
    }
    if(fetch){
        kdebug(DNONE,"Instruction fetch\n");
    }
    end:
    kdebug(DNONE,"#===========Registers==========#\n");
    kdebug(DNONE,"RAX: 0x%lx RCX: 0x%lx RBX: 0x%lx\n",reg.rax,reg.rcx,reg.rbx);
    kdebug(DNONE,"RDI: 0x%lx RSI: 0x%lx RDX: 0x%lx\n",reg.rdi,reg.rsi,reg.rdx);
    kdebug(DNONE,"RIP: 0x%lx RBP: 0x%lx RSP: 0x%lx\n",reg.rip,reg.rbp,reg.rsp);
    kdebug(DNONE,"R08: 0x%lx R09: 0x%lx R10: 0x%lx\n",reg.r8,reg.r9,reg.r10);
    kdebug(DNONE,"R11: 0x%lx R12: 0x%lx R13: 0x%lx\n",reg.r11,reg.r12,reg.r13);
    kdebug(DNONE,"R14: 0x%lx R15: 0x%lx\n",reg.r14,reg.r15);
    kdebug(DNONE,"CS: 0x%lx SS: 0x%lx EC: 0x%lx\n",reg.cs,reg.ss,reg.ec);
    kdebug(DNONE,"RFLAGS: 0x%lx\n",reg.rflags);
    kdebug(DNONE,"#==============================#\n");
    for(;;){asm("hlt");}
}