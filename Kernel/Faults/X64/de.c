#include <stdint.h>
#include <debug.h>
#include <registers.h>
#include "interrupt.h"

intHandler divByZero(registers reg){
    for(;;){asm("hlt");}
}