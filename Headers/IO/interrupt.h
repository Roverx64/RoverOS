#pragma once

#include "types.h"
#include "registers.h"

//For fault handlers

typedef void (*irh)(regs);
extern void irqHandler(int interrupt, regs r);
extern void isrHandler(regs r, uint32 ec);