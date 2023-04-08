#include <stdint.h>
#include <debug.h>
#include <registers.h>
#include "interrupt.h"

extern intHandler generalProtectionFault(registers reg);
extern intHandler divByZero(registers reg);
extern intHandler debugException(registers reg);
extern intHandler pageFault(registers reg);

void initFaultHandlers(){
    addInterruptHandler(0x0,divByZero);
    addInterruptHandler(0x1,debugException);
    addInterruptHandler(0xD,generalProtectionFault);
    addInterruptHandler(0xE,pageFault);
}