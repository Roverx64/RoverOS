#include <stdint.h>
#include <string.h>

void *memset(void *ptr, int c, size bytes){
    int *dest = (int*)ptr;
    bytes = bytes/sizeof(int);
    for(int i = 0; i < bytes; ++i){
        dest[i] = c;
    }
    return ptr;
}