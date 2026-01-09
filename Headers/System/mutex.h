#pragma once
#include <stdint.h>

typedef uint8_t mutex_t;
#define GET_LOCK(lock) while(lock&0x1){} lock |= 0x1
#define FREE_LOCK(lock) lock = 0x0