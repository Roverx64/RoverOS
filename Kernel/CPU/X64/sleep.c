#include <stdint.h>
#include "sleep.h"

void sleep(uint64 ms){
    for(uint64 b = 0; b < ms; ++b){asm("hlt");}
}