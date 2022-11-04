#include "types.h"
#include "boot.h"

struct bootInfoS *bootInfo;

void kmain(uintptr *bi){
    bootInfo = (struct bootInfoS*)bi;
    asm("cli");
    for(;;){asm("hlt");}
    return;
}