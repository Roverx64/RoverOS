#include <stdint.h>
#include <debug.h>
#include <cpu.h>

intHandler invalidOpcode(registers reg){
    kdebug(DNONE,"#===Invalid=Opcode===#\n");
    kdebug(DNONE,"RIP: 0x%lx\n",reg.rip);
    kdebug(DNONE,"#====================#\n");
    for(;;){asm("hlt");}
}