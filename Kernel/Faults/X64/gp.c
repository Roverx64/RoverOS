#include <stdint.h>
#include <debug.h>
#include <registers.h>
#include "interrupt.h"

intHandler generalProtectionFault(registers reg){
    kdebug(DNONE,"#======General=Protection=Fault======#\n");
    kdebug(DNONE,"Error Code: 0x%llx\n",reg.ec);
    kdebug(DNONE,"#====================================#\n");
    for(;;){asm("hlt");}
}