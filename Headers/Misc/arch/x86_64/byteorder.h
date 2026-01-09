#pragma once


#define bswap64(b) (asm volatile("bswap %0"::))
#define bswap32(b) (asm volatile("bswap %0"::))
#define bswap16(b) (asm volatile("bswap %0"::))