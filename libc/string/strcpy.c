#include <stdint.h>
#include <string.h>

void *strcpy(char *restrict s1, char *restrict s2){
    while(*s2 != '\0'){
        *s1 = *s2;
        ++s2;
        ++s1;
    }
    return (void*)s1;
}