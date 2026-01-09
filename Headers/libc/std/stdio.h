#pragma once
#include <stdint.h>

#define SEEK_SET 0
#define SEEK_END 2
#define SEEK_CUR 1

typedef struct FILE{
    void *stream;
    uint64_t offset;
    uint64_t size;
}FILE;

extern uint64_t ftell(FILE *fl);
extern size_t fread(void *dest, size_t bytes, size_t n, FILE *fl);
extern uint64_t fseek(FILE *fl,uint64_t off,int seek);