#pragma once
#include <stdint.h>
#include "paging.h"

extern uint64 loadElf(pageSpace *procmap, void *elf);