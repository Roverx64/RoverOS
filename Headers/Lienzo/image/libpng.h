#pragma once

#include <stdint.h>
#include <vfs.h>

#define PNG_SIGNATURE 0x89504E470D0A1A0A

struct pngSignature{
    uint64_t signature;
}__attribute__((packed));

extern bool checkPNG(vFILE *fl);