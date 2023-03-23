#include <stdint.h>
#include <debug.h>
#include <registers.h>
#include "interrupt.h"

intHandler debugException(registers reg){
    for(;;){asm("hlt");}
}