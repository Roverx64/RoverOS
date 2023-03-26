#pragma once
#include <stdint.h>
#include "bootinfo.h"

#define SUBENTRY_MASK 0xFF0000

#define GET_PMM_ENTRY(p) ((p/PAGE_SZ)%64)
#define GET_PMM_SUBENTRY(p) ((p&SUBENTRY_MASK)/PAGE_SZ)

extern void initPMM(struct bootInfo *kinf);