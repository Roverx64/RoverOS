#include <stdint.h>
#include <com.h>
#include <debug.h>
#include <stdarg.h>
#define CAP_MASK 0xDF //& this to capitalize a 8 bit char

char ch = 'A';


//'a' 01100001
//'A' 01000001
//    11011111


void writeHex(uint64 num,uint16 bytes,bool caps){
    uint8 nibble = 0x0;
    bool leading = true;
    uint16 bits = (bytes*8);
    uint8 ch = 0x0;
    uint64 mask = (uint64)0xF000000000000000;
    for(int i = 0;i < bits;i+=4){
        nibble = (uint8)((uint64)(num&mask)>>(bits-(i+4)));
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

void kdebug(const char *func, uint8 level, const char *str, ...){
    uint8 sizeModifier = 0; //u8 1, u32 4, u64 8
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
    writeString(KDEBUG_PORT,func);
    writeByte(KDEBUG_PORT,'[');
    writeByte(KDEBUG_PORT,dchar);
    writeByte(KDEBUG_PORT,']');
    writeByte(KDEBUG_PORT,':');
    skip:
    for(int i = 0; str[i] != '\0'; ++i){
        if(str[i] != '%'){
            writeByte(KDEBUG_PORT,str[i]);
            continue;
        }
        loop:
        ++i;
        switch(str[i]){
            case 'i':
            break;
            case 'X':
            caps = true;
            case 'x':
            writeHex(va_arg(args,uint64),sizeof(uint64),caps);
            break;
            case 'c':
            writeByte((char)KDEBUG_PORT,va_arg(args,int));
            break;
            case 'h':
            sizeModifier = 1;
            goto loop;
            break;
            case 'l':
            sizeModifier += 4;
            goto loop;
            break;
            case 's':
            writeString(KDEBUG_PORT,va_arg(args,char*));
            break;
            default:
            break;
        }
        sizeModifier = 0;
    }
}