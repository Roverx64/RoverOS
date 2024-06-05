#pragma once

#include <stdint.h>

extern char *lltoa(int64_t n, char *buffer, int base);
extern char *ltoa(long int n, char *buffer, int base);
extern char *itoa(int n, char *buffer, int base);