#include <stdlib.h>
#include <stdint.h>

char *lltoa(int64_t n, char *buffer, int base){
    return buffer;
}

char *ltoa(long int n, char *buffer, int base){
    return lltoa((int64_t)n,buffer,base);
}

char *itoa(int n, char *buffer, int base){
    return lltoa((int64_t)n,buffer,base);
}

//102235
//11000111101011011