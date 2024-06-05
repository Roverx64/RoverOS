#include <stdint.h>
#include <string.h>

//TODO: Move these into assembly files so GCC can't ruin them

void *memcpy(void *dest, void *src, size_t len){
    for(;len > 0;--len){
        ((uint8_t*)dest)[len] = ((uint8_t*)src)[len];
    }
    return dest;
}
