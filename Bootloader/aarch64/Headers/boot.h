#pragma once

#include <stdint.h>
#include <mmap.h>

//mem.c
extern void *allocMem(size_t sz);
extern void addMmapEntry(uint64_t base, size_t len, uint64_t type);