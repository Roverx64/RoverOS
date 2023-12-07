#include <stdint.h>
#include <string.h>

//TODO: Move these into assembly files so GCC can't ruin them

void *memcpy(void *dest, int n, void *src){
    for(;n > 0;--n){
        ((uint8*)dest)[n] = ((uint8*)src)[n];
    }
    return dest;
}