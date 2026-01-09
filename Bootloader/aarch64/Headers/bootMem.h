#pragma once
#include <stdint.h>

extern void addMmapEntry(uint64_t phys, uint64_t virt, size_t len, uint64_t type);
extern void addAllocation(uint64_t phys, uint64_t virt, size_t sz, uint64_t type);
extern void *allocMem(size_t sz, bool aligned);