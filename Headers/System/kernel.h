#pragma once

#define OS_NAME "RoverOS"
#define OS_VERSION_MAJOR 1
#define OS_VERSION_MINOR 0

#define KERNEL_NAME "Fortuna"
#define KERNEL_VERSION_MAJOR 0
#define KERNEL_VERSION_MINOR 1

#define KERNEL_LOAD_BASE 0xFFFFFFFF80000000


extern void kpanicInt(const char *name, uint64_t line,const char *str, uint64_t ex);
#define kpanic(str,ex) kpanicInt(__FUNCTION__,__LINE__,str,ex)
