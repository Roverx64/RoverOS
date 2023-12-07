#pragma once

typedef struct{
    unsigned long arg;
    unsigned long value;
    void *func;
}bootArg;

#define ARG_DUMP_VGA 0x43F
#define ARG_NO_JUMP 0x39E
#define ARG_DUMP_CR3 0x3EB
#define ARG_LOAD_CONSOLE 0x640
#define ARG_TEST_DRAW 0x475
#define ARG_DUMP_PML 0x452
#define ARG_MAP_MMAP 0x40C
#define ARG_END 0xFFFFFFFFFFFFFFFF

#define VALUE_TRUE 0x53
#define VALUE_FALSE 0x50
#define VALUE_NULL 0x0