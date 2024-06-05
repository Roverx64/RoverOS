#include <stdint.h>
#include <stdbool.h>
#include <com.h>
#include <debug.h>
#include <stdarg.h>
#include <kmalloc.h>
#define CAP_MASK 0xDF //& this to capitalize a 8 bit char

char ch = 'A';


void writeHex(uint64_t num,uint16_t bytes,bool caps){
    uint8_t nibble = 0x0;
    bool leading = true;
    uint16_t bits = (bytes*8);
    uint8_t ch = 0x0;
    uint64_t mask = (uint64_t)0xF000000000000000;
    for(int i = 0;i < bits;i+=4){
        nibble = (uint8_t)((uint64_t)(num&mask)>>(bits-(i+4)));
        mask = mask>>4;
        if(leading && nibble == 0x0){continue;}
        leading = false;
        if(nibble >= 0xA){
            ch = nibble+87;
            if(caps){ch &= CAP_MASK;}
        }
        else{
            ch = nibble|0x30;
        }
        writeByte(KDEBUG_PORT,ch);
        ch = 0x0;
    }
    if(leading){writeByte(KDEBUG_PORT,'0');}
}

void kprintChar(char ch){
    writeByte(KDEBUG_PORT,ch);
}

void kprintString(const char *str){
    for(int i = 0; str[i] != '\0';++i){
        kprintChar(str[i]);
    }
}

void writeInt(uint64_t val){
    char ch = 0x30;
}

void kdebug(const char *func, uint8_t level, const char *str, ...){
    uint8_t sizeModifier = 0; //u8 1, u32 4, u64 8
    va_list args;
    va_start(args,str);
    char dchar = 'g';
    bool caps = false;
    switch(level){
        case DLEVEL_INFO:
        dchar = 'i';
        break;
        case DLEVEL_WARN:
        dchar = 'w';
        break;
        case DLEVEL_ERROR:
        dchar = 'e';
        break;
        case DLEVEL_FATAL:
        dchar = 'f';
        break;
        case DLEVEL_NONE:
        goto skip;
        break;
    }
    kprintString(func);
    kprintChar('[');
    kprintChar(dchar);
    kprintChar(']');
    kprintChar(':');
    skip:
    for(int i = 0; str[i] != '\0'; ++i){
        if(str[i] != '%'){
            kprintChar(str[i]);
            continue;
        }
        loop:
        ++i;
        switch(str[i]){
            case 'i':
            break;
            case 'X':
            caps = true;
            writeHex(va_arg(args,uint64_t),sizeModifier,caps);
            break;
            case 'x':
            writeHex(va_arg(args,uint64_t),sizeModifier,caps);
            break;
            case 'c':
            kprintChar(va_arg(args,int));
            break;
            case 'h':
            sizeModifier = 1;
            goto loop;
            break;
            case 'l':
            sizeModifier = 8;
            goto loop;
            break;
            case 's':
            kprintString(va_arg(args,char*));
            break;
            default:
            break;
        }
        sizeModifier = 0;
    }
}

//Setup UI debugging
void initUIDebug(void *buffer, uint32_t Xr, uint32_t Yr){
}