#include <stdint.h>
#include <stdbool.h>
#include <com.h>
#include <kprint.h>
#include <stdarg.h>
#include <kmalloc.h>
#define CAP_MASK 0xDF //& this to capitalize a 8 bit char

char ch = 'A';

void writeHex(uint64_t num,uint16_t bytes,bool caps){
    uint8_t nibble = 0x0;
    bool leading = true;
    uint16_t bits = (bytes*8);
    uint8_t ch = 0x0;
    uint64_t mask = ((uint64_t)0xF000000000000000)>>(64-bits);
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

char revbuff[20]; //64 bit limit
void writeUint(uint64_t val){
    if(val == 0){kprintChar('0'); return;}
    if(val > 9){
        kprintChar((char)((char)val|'0'));
        return;
    }
    uint64_t rem = val%10;
    uint8_t idx = 0;
    while(val != 0){
        uint64_t rem = val%10;
        revbuff[idx] = rem|0x30;
        ++idx;
        val -= rem;
    }
    while(idx){
        kprintChar(revbuff[idx]);
        --idx;
    }
}

void writeInt(int64_t val){
    if(val < 0){
        kprintChar('-');
        val = val*-1;
    }
    writeUint((uint64_t)val);
}

void kprintf(const char *func, uint8_t level, const char *str, ...){
    uint8_t sizeModifier = sizeof(int); //u8 1, u32 4, u64 8
    va_list args;
    va_start(args,str);
    char dchar = 'g';
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
            if(sizeModifier == sizeof(uint64_t)){
                writeInt(va_arg(args,uint64_t));
            }else{writeInt(va_arg(args,uint32_t));}
            break;
            case 'X':
            writeHex(va_arg(args,uint64_t),sizeModifier,true);
            break;
            case 'x':
            writeHex(va_arg(args,uint64_t),sizeModifier,false);
            break;
            case 'c':
            kprintChar(va_arg(args,int));
            break;
            case 'h':
            sizeModifier = sizeof(uint8_t);
            goto loop;
            break;
            case 'l':
            sizeModifier = sizeof(uint64_t);
            goto loop;
            break;
            case 's':
            kprintString(va_arg(args,char*));
            break;
            case 'u':
            if(sizeModifier == sizeof(uint64_t)){
                writeUint(va_arg(args,uint64_t));
            }else{writeUint(va_arg(args,uint32_t));}
            break;
            default:
            break;
        }
        sizeModifier = sizeof(int);
    }
}