#pragma once

#include <stdint.h>

#define PSF1_MAGIC 0x0436
#define PSF2_MAGIC 0x864AB572

#define PSF1_MODE_512 0x1
#define PSF1_MODE_HAS_TAB 0x2
#define PSF1_MODE_HAS_SEQ 0x4
#define PSF1_MAX_MODE 0x5
#define PSF1_SEPARATOR  0xFFFF
#define PSF1_START_SEQ   0xFFFE
#define PSF2_HAS_UNICODE_TABLE 0x01
#define PSF2_MAXVERSION 0
#define PSF2_SEPARATOR  0xFF
#define PSF2_START_SEQ   0xFE

typedef struct{
    uint16 magic;
    uint8 mode;
    uint8 characterSz;
}PSF1Header __attribute__((packed));

typedef struct{
    uint32 magic;
    uint32 version;
    uint32 headerSz;
    uint32 flags;
    uint32 lenght;
    uint32 characterSz;
    uint32 height;
    uint32 width;
}PSF2Header __attribute__((packed));