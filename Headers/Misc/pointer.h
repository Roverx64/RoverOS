#pragma once

#include <stdint.h>

#define MAX_POINTER (uint64_t)0xFFFFFFFFFFFFFFFF
#define MAX_ALIGNED_POINTER (uint64_t)0xFFFFFFFFFFFFF000

#define MIN_ADDR_1MB 0x0
#define MIN_ADDR_LOW 0x100000
#define MIN_ADDR_HGH 0x100000000

//Miscellanious pointer macros

#define BYTES_TO_ALIGN_PTR(ptr,align) ((align-((uint64_t)ptr%align))) //!< Bytes needed to align a pointer upwards

#define ALIGN_PTR_UP(ptr,align) (((uint64_t)ptr+BYTES_TO_ALIGN_PTR(ptr,align))) //!< Returns an upwards aligned pointer
#define ALIGN_PTR_DOWN(ptr,align) () //!< Returns a downwards aligned pointer