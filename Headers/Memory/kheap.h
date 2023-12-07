#pragma once

#include <stdint.h>

//Simple linked list for the kheap

#define KHEAP_MAGIC 0xFEEF

typedef struct{
    uint16_t magic;
    uint32_t allocated;
    uint8_t boundary;
    uint32_t free;
    void *prev;
    void *next;
}kheapBlock;

extern void *halloc(size sz);
extern void hfree(void *ptr);