#pragma once

#include <stdint.h>

static inline void outb(uint16_t p, uint8_t b){
    asm volatile("out %1, %0"::"d"(p), "aN"(b):);
}

static inline uint8_t inb(uint16_t p){
    uint8_t b = 0;
    asm volatile("in %1, %0":"=a"(b):"dN"(p):);
    return b;
}

static inline void outd(uint16_t p, uint32_t d){
    asm volatile("out %1, %0"::"d"(p), "a"(d):);
}

static inline uint32_t ind(uint16_t p){
    uint32_t d = 0;
    asm volatile("in %1, %0":"=a"(d):"d"(p):);
    return d;
}