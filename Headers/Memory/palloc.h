#pragma once

#include <stdint.h>

extern void *palloc(uint64 sz);
extern void pfree(void *ptr);