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
    struct pde page;
    getPageInfo((pageSpace*)cr3,&page,faultAddr);
    uint8 exists = page.a1;
    if(!exists){goto cause;}
    kdebug(DNONE,"PHYS=0x%llx\n",page.addr);
    kdebug(DNONE,"FLAGS=");
    if(page.present){kdebug(DNONE,"P");}else{kdebug(DNONE,"-");}
    if(page.write){kdebug(DNONE,"W");}else{kdebug(DNONE,"R");}
    if(page.user){kdebug(DNONE,"U");}else{kdebug(DNONE,"S");}
    if(page.pwt){kdebug(DNONE,"T");}else{kdebug(DNONE,"-");}
    if(page.pcd){kdebug(DNONE,"C");}else{kdebug(DNONE,"-");}
    if(page.accessed){kdebug(DNONE,"A");}else{kdebug(DNONE,"-");}
    if(page.dirty){kdebug(DNONE,"D");}else{kdebug(DNONE,"-");}
    if(page.global){kdebug(DNONE,"G");}else{kdebug(DNONE,"-");}
    if(page.pat){kdebug(DNONE,"O");}else{kdebug(DNONE,"-");}
    if(page.nx){kdebug(DNONE,"-");}else{kdebug(DNONE,"X");}
    kdebug(DNONE,"\n");
    cause:
    //Determine cause
    kdebug(DNONE,"Cause: ");
    if(resv){
        kdebug(DNONE,"Wrote 1 to reserved bit(s)\n");
        goto end;
    }
    if(!exists){
        kdebug(DNONE,"Non-existent page\n");
        goto end;
    }
    if(!present){
        if(write){kdebug(DNONE, "Wrote to ");}
        else{kdebug(DNONE,"Read from ");}
        kdebug(DNONE,"unmapped page\n");
        goto end;
    }
    if(fetch){
        kdebug(DNONE,"Instruction fetch\n");
    }
    end:
    kdebug(DNONE,"#==============================#\n");
    for(;;){asm("hlt");}
}