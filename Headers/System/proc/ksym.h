#pragma once

#include <stdint.h>
#include <elf.h>

extern ELF64Sym *getSymbol(uint64_t ptr);
extern char *symbolToName(ELF64Sym *sym);