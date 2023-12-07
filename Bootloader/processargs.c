#include "polarboot.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

void writeArg(FILE *dst, uint64_t arg, uint64_t value){
    bootArg barg;
    barg.arg = (unsigned long)arg;
    barg.value = (unsigned long)value;
    fwrite(&barg,sizeof(barg),1,dst);
}

uint64_t readArg(FILE *src){
    //Good enough to avoid collisions for now
    uint64_t v = 0x0;
    bool swp = false;
    char ch = 0x0;
    for(;ch != '=';ch = fgetc(src)){
        if(ch == '='){break;}
        if(ch == 0x0A){continue;}
        if(ch == EOF){return ARG_END;}
        printf("%c",ch);
        if(swp){v += (ch<<1)^ch; swp = false;}
        else{swp = true; v += ch;}
    }
    printf("=0x%lx\n",v);
    return v;
}

uint64_t readValue(FILE *src){
    uint64_t v = 0x0;
    bool swp = false;
    char ch = 0x0;
    for(;ch != ';';ch = fgetc(src)){
        if(ch == 0x0A){continue;}
        if(ch == EOF){return ARG_END;}
        printf("%c",ch);
        if(swp){v += (ch<<1)^ch; swp = false;}
        else{swp = true; v += ch;}
    }
    printf("=0x%lx\n",v);
    return v;
}

int main(){
    FILE *pre = fopen("./Other/polarboot.preargs","r");
    FILE *post = fopen("./Other/polarboot.args","w+");
    for(;;){
        uint64_t arg = readArg(pre);
        if(arg == ARG_END){
            writeArg(post,ARG_END,ARG_END);
            goto end;
        }
        uint64_t val = readValue(pre);
        writeArg(post,arg,val);
    }
    end:
    fclose(post);
    fclose(pre);
}