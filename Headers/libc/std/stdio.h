#pragma once
#include <stdint.h>

#define SEEK_SET 0
#define SEEK_END 2
#define SEEK_CUR 1

typedef struct FILE{
    void *stream;
    uint64 offset;
    uint64 size;
}FILE;

extern uint64 ftell(FILE *fl);
extern size fread(void *dest, size bytes, size n, FILE *fl);
extern uint64 fseek(FILE *fl,uint64 off,int seek);