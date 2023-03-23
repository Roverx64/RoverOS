#include <stdint.h>
#include <port.h>

void outb(uint16 p, uint8 b){
    asm volatile("outb %1, %0" : : "dN" (p), "a" (b));
}

uint8 inb(uint16 p){
    uint8 r;
    asm volatile("inb %1, %0" : "=a" (r) : "dN" (p));
    return r;
}