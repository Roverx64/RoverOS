#include <stdint.h>

#define DLEVEL_INFO 1
#define DLEVEL_WARN 2
#define DLEVEL_ERROR 3
#define DLEVEL_FATAL 4
#define DLEVEL_NONE 5

#define DGENERAL 0
#define DINFO __FUNCTION__, DLEVEL_INFO
#define DWARN __FUNCTION__, DLEVEL_WARN
#define DERROR __FUNCTION__, DLEVEL_ERROR
#define DFATAL __FUNCTION__, DLEVEL_FATAL
#define DNONE __FUNCTION__, DLEVEL_NONE

extern void kdebug(const char *func, uint8 level, const char *str, ...);