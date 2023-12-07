#include <stdint.h>
#include <debug.h>
#include <registers.h>
#include "interrupt.h"

intHandler generalProtectionFault(registers reg){
    kdebug(DNONE,"#======General=Protection=Fault======#\n");
    kdebug(DNONE,"Error Code: 0x%llx\n",reg.ec);
    kdebug(DNONE,"Fault occured ");
    if(reg.ec&0x1){kdebug(DNONE,"externally in ");}
    else{kdebug(DNONE,"locally in ");}
    switch((reg.ec>>1)&0x3){
        case 0x0: //GDT
        kdebug(DNONE,"GDT");
        break;
        case 0x3: //IDT
        case 0x1: //IDT
        kdebug(DNONE,"IDT");
        break;
        case 0x2: //LDT
        kdebug(DNONE,"LDT");
        break;
    }
    kdebug(DNONE," entry #0x%x\n",(uint32)(reg.ec>>3)&0x1FFF);
    //dumpRegisters(reg);
    readInstruction(reg.rip);
    kdebug(DNONE,"#====================================#\n");
    for(;;){asm("hlt");}
}
//1111111111111