#include <stdint.h>
#include <stdio.h>

//NOTE: I must wait until this can be made in userspace

uint64 ftell(FILE *fl){
    return fl->offset;
}

size fread(void *dest, size bytes, size n, FILE *fl){
    for(size i = 0;i < bytes;++i){((uint8*)dest)[i] = ((uint8*)fl->stream)[i+fl->offset];}
    fl->offset += bytes*n;
    return bytes;
}

uint64 fseek(FILE *fl,uint64 off,int seek){
    if(seek == SEEK_SET){fl->offset = off;}
    if(seek == SEEK_CUR){fl->offset += off;}
    if(seek == SEEK_END){fl->offset = (fl->size-off);}
    return 0;
}