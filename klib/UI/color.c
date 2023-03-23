#include <color.h>

uint32 color(uint8 r, uint8 g, uint8 b){
    uint32 col = 0x0;
    return ((((col|r)<<8)|g)<<8)|b;
}