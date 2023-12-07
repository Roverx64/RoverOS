#pragma once

#define OS_NAME "RoverOS"
#define OS_VERSION_MAJOR 1
#define OS_VERSION_MINOR 0

#define KERNEL_NAME "Fortuna"
#define KERNEL_VERSION_MAJOR 0
#define KERNEL_VERSION_MINOR 1

#define kpanic(str,ex) kpanicInt(__FUNCTION__,__LINE__,str,ex)

struct sysinfoTime{
    //Deadline
    bool hasDeadline;
    uint64 deadlinetpms; //Ticks per MS
    //Lapic
    bool hasLapic;
    uint64 lapictpms;
    //HPET
    bool hasHpet;
    uint64 hpettpms;
};

typedef struct{
    struct sysinfoTime time;
}sysinfo;

extern sysinfo sysinf;