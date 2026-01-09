#pragma once

#include <stdint.h>
#include <paging.h>

#define OS_NAME "RoverOS"
#define OS_VERSION_MAJOR 1
#define OS_VERSION_MINOR 0

#define KERNEL_NAME "Fortuna"
#define KERNEL_VERSION_MAJOR 1
#define KERNEL_VERSION_MINOR 0

#define KERNEL_LOAD_BASE 0xFFFFFFFF80000000

extern void kpanicInt(const char *func, const char *file, const uint64_t line, const char *str, uint64_t ex);
extern void kassertInt(const char *func, const char *file, const uint64_t line, int expression, const char *fail);
#define kpanic(str,ex) kpanicInt(__FUNCTION__,__FILE__,__LINE__,str,ex)
#define KASSERT(expression, fail) kassertInt(__FUNCTION__,__FILE__,__LINE__,expression,fail)

extern pageSpace kspace; //paging.c