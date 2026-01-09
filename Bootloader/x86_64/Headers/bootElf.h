#pragma once
#include <stdint.h>
#include <stdbool.h>

extern uint64_t getELFMemSize(EFIFile *elf);
extern uint64_t loadELF(EFIFile *elf, bool kernel);