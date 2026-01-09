#pragma once


#define bswap64(b) (asm volatile("REV64 %0"::))
#define bswap32(b) (asm volatile("REV32 %0"::))
#define bswap16(b) (asm volatile("REV16 %0"::))