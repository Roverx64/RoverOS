#include <stdint.h>
#include <kprint.h>
#include <fdebug.h>
#include <task.h>
#include <paging.h>
#include <cpu.h>
#include <vmm.h>

void generalProtectionFault(registers *reg){
    knone("#======General=Protection=Fault======#\n");
    knone("Error Code: 0x%llx\n",reg->ec);
    if(reg->ec == 0x0){
        knone("Fault cause by non-segment related issue\n");
        goto end;
    }
    knone("Fault occured ");
    if(reg->ec&0x1){knone("externally in ");}
    else{knone("locally in ");}
    switch((reg->ec>>1)&0x3){
        case 0x0: //GDT
        knone("GDT");
        break;
        case 0x3: //IDT
        case 0x1: //IDT
        knone("IDT");
        break;
        case 0x2: //LDT
        knone("LDT");
        break;
    }
    knone(" entry #0x%x\n",(uint32_t)(reg->ec>>3)&0x1FFF);
    end:
    //Check procmap with CR2
    uint64_t cr2 = 0x0;
    asm volatile("movq %%cr2, %0" : "=r"(cr2));
    knone("Potential fault addr: 0x%lx\n",cr2);
    uint64_t phys = virtToPhys(cr2);
    if(phys){knone("Addr is mapped to 0x%lx\n",phys);}
    else{knone("Address is not mapped\n");}
    knone("#===Virtual=Mem===#\n");
    dumpVMarker(currentProcessVMarker());
    dumpRegisters(reg);
    //readInstruction(reg->rip);
    for(;;){asm("hlt");}
}
//1111111111111
