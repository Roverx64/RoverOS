#pragma once
#include <stdint.h>

#define TSK_PRIORITY_KERNEL 0
#define TSK_PRIORITY_HIGH 1
#define TSK_PRIORITY_MEDIUM 2
#define TSK_PRIORITY_LOW 3

#define TS_CALC(tps,priority) (tps/((priority+1)*60))

typedef struct{
    uint16 timeSlice;
    
}ticket;

typedef struct{
    ticket *tickets;
}TaskQueue;

typedef struct{
    
    
}CPUQueue;