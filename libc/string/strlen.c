#include <stdint.h>
#include <string.h>

size strlen(const char *str){
    size i = 0;
    for(;str[i] != '\0';++i){}
    return i;
}