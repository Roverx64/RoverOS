#pragma once
#include <stdint.h>

typedef uint8_t mutex_t;
#define GET_LOCK(lock) while(lock&0x1){} lock |= 0x1
#define FREE_LOCK(lock) lock = 0x0

extern void *kmalloc(size_t sz);
extern void kfree(void *ptr);
extern void *krealloc(void *ptr, size_t sz);