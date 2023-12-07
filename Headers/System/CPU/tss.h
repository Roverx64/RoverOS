#pragma once

#include <stdint.h>

struct TSS{
    uint32_t res0;
    uint64_t RSP0;
    uint64_t RSP1;
    uint64_t RSP2;
    uint64_t res1;
    uint64_t IST0;
    uint64_t IST1;
    uint64_t IST2;
    uint64_t IST3;
    uint64_t IST4;
    uint64_t IST5;
    uint64_t IST6;
    uint64_t IST7;
    uint64_t res2;
    uint32_t res3;
    uint32_t iopb;
}__attribute__((packed));

extern void loadTSS(uint8_t segment);