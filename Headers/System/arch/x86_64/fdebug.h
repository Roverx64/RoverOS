#pragma once

#include <cpu.h>

#define INS_LEGACY_LOCK 0xF0
#define INS_LEGACY_REPN 0xF2
#define INS_LEGACY_REP 0xF3
#define INS_LEGACY_CS 0x2E
#define INS_LEGACY_SS 0x36
#define INS_LEGACY_DS 0x3E
#define INS_LEGACY_ES 0x26
#define INS_LEGACY_FS 0x64
#define INS_LEGACY_GS 0x65
#define INS_LEGACY_BRT 0x2E
#define INS_LEGACY_BRNT 0x3E
#define INS_LEGACY_OPS 0x66
#define INS_LEGACY_ADS 0x67

#define INS_OPCODE_PREFIX 0x0F
#define INS_OPCODE_PREFIX_0 0x38
#define INS_OPCODE_PREFIX_1 0x3A

#define INS_REX_FIXED(rex) (rex>>4)
#define INS_REX_OPSZ(rex) ((rex>>3)&0x1)
#define INS_REX_MODRM_REG(rex) ((rex>>2)&0x1)
#define INS_REX_SIB(rex) ((rex>>1)&0x1)
#define INS_REX_MODRM_RM(rex) (rex&0x1)

#define INS_IS_REX(b) ((b>>4)&0x4) //!< 0 if not REX byte


struct instruction{
    uint8_t rex;
    uint8_t opcode;
    uint8_t opPrefix;
    uint8_t opPrefix0;
};

struct stackFrame{
    uint64_t framePointer;
    uint64_t ret;
}__attribute__((packed));

extern void readInstruction(uint64_t rip);
extern void dumpRegisters(registers *reg);
extern void stackTrace(uint64_t rbp);