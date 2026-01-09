#pragma once
#include <stdint.h>

#define DLEVEL_INFO 1
#define DLEVEL_WARN 2
#define DLEVEL_ERROR 3
#define DLEVEL_FATAL 4
#define DLEVEL_NONE 5

#define DGENERAL __FUNCTION__, 0
#define DINFO __FUNCTION__, DLEVEL_INFO
#define DWARN __FUNCTION__, DLEVEL_WARN
#define DERROR __FUNCTION__, DLEVEL_ERROR
#define DFATAL __FUNCTION__, DLEVEL_FATAL
#define DNONE __FUNCTION__, DLEVEL_NONE

//NOTE: ##__VA_ARGS__ is GCC specific
//Nice to use macros
#define kinfo(str,...) kprintf(DINFO,str,##__VA_ARGS__)
#define kwarn(str,...) kprintf(DWARN,str,##__VA_ARGS__)
#define kerror(str,...) kprintf(DERROR,str,##__VA_ARGS__)
#define kfatal(str,...) kprintf(DFATAL,str,##__VA_ARGS__)
#define knone(str,...) kprintf(DNONE,str,##__VA_ARGS__)

extern void kprintf(const char *func, uint8_t level, const char *str, ...);
extern void kprintChar(char ch);
