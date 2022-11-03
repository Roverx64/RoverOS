#include "types.h"
#include "boot.h"

void kmain(){
    asm("cli");
    for(;;){asm("hlt");}
    return;
}