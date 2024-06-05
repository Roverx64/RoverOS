#include <stdint.h>
#include <math.h>

size_t diff(size_t a, size_t b){
    return (a > b) ? (a-b):(b-a);
}