#pragma once
#include <registers.h>

typedef (*intHandler)(registers);

extern void addInterruptHandler(uint32 vector, intHandler handler);