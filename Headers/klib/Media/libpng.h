#pragma once
#include <stdint.h>

//PNG is in network (big endian) order
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#include <byteswap.h>
#define rorder64(u64) bswap_64(u64)
#define rorder32(u32) bswap_32(u32)
#define rorder16(u16) bswap_16(u16)
#else
//Empty macros since big endian machines do not need to swap the byte order
#define rorder64(u64) (u64)
#define rorder32(u32) (u32)
#define rorder16(u16) (u16)
#endif

#define PNG_SIGNATURE 0x89504E470D0A1A0A

//Color types
#define PNG_COLOR_GREYSCALE 0
#define PNG_COLOR_RGB 2
#define PNG_COLOR_PALETTE 3
#define PNG_COLOR_GRAYALPHA 4
#define PNG_COLOR_RGBA 6

//Chunk types
#define PNG_CHUNK_IHDR 0x49484452
#define PNG_CHUNK_IDAT 0x49444154
#define PNG_CHUNK_IEND 0x49454E44
#define PNG_CHUNK_PLTE 0x0

//Compression
#define PNG_COMPRESSION_STANDARD 0

typedef struct pngSignature{
    uint64 signature;
}pngSignature;

struct chunkIdent{
    uint32 length;
    uint32 type;
}__attribute__((packed));

typedef struct pngIHDR{
    struct chunkIdent ident;
    uint32 width;
    uint32 height;
    uint8 depth;
    uint8 colorType;
    uint8 compression;
    uint8 filter;
    uint8 interlace;
}pngIHDR;

typedef struct pngIDAT{
    struct chunkIdent ident;
}pngIDAT;

typedef struct pngIEND{
    struct chunkIdent ident;
}pngIEND;

extern void readPNG(void *data);

#define NEXT_CHUNK(ident) ((struct chunkIdent*)((uint64)ident+ident->length+sizeof(struct chunkIdent)+4))