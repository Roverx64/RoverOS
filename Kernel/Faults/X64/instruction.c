#include <stdint.h>
#include <debug.h>
#include <registers.h>
#include "instruction.h"

void printOperand(uint8 op, bool qword, bool addr){
    if(addr){kdebug(DNONE,"[");}
    if(qword){kdebug(DNONE,"R");}else{kdebug(DNONE,"E");}
    switch(op){
        case 0:
        kdebug(DNONE,"AX");
        break;
        case 1:
        kdebug(DNONE,"CX");
        break;
        case 2:
        kdebug(DNONE,"DX");
        break;
        case 3:
        kdebug(DNONE,"BX");
        break;
        case 4:
        kdebug(DNONE,"SP");
        break;
        case 5:
        kdebug(DNONE,"BP");
        break;
        case 6:
        kdebug(DNONE,"SI");
        break;
        case 7:
        kdebug(DNONE,"DI");
        break;
    }
    if(addr){kdebug(DNONE,"]");}
}

void readInstruction(uint64 rip){
    uint8 *ins = (uint8*)rip;
    kdebug(DNONE,"0x%lx: ",rip);
    uint8 rex = ins[0];
    uint8 opcode = 0x0;
    //Opcode
    if((rex>>4) == 0x4){
        kdebug(DNONE,"REX[");
        if(rex&INS_REX_MODRM){kdebug(DNONE,"M");}else{kdebug(DNONE,"-");}
        if(rex&INS_REX_INDEX){kdebug(DNONE,"I");}else{kdebug(DNONE,"-");}
        if(rex&INS_REX_REG){kdebug(DNONE,"R");}else{kdebug(DNONE,"-");}
        if(rex&INS_REX_WIDE){kdebug(DNONE,"Q] ");}else{kdebug(DNONE,"D] ");}
        opcode = ins[1];
    }else{kdebug(DNONE,"0x%hx-",ins[0]);}
    if((opcode == 0xC7) || (opcode == 0x89)){
        kdebug(DNONE,"MOV");
        if(rex&INS_REX_WIDE){kdebug(DNONE,"Q");}
        kdebug(DNONE," ");
    }
    //Registers
    printOperand(ins[2],true,false);
    kdebug(DNONE,", ");
    printOperand(ins[3],true,false);
    kdebug(DNONE,"\n");
    kdebug(DNONE,"0x%hx-0x%hx-0x%hx-0x%hx-0x%hx-0x%hx-0x%hx-0x%hx\n",ins[0],ins[1],ins[2],ins[3],ins[4],ins[5],ins[6],ins[7]);
}

//[11|001|000]
//[MD|REG|RM ]
