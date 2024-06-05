#pragma once

#include <stdint.h>

extern size strlen(const char *str);
extern void *memset(void *ptr, int c, size_t bytes);
extern void *memcpy(void *dest, void *src, size_t len);
extern void *strcpy(char *restrict s1, char *restrict s2);