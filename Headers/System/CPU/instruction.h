#pragma once

#define INS_REX_FIXED 0x4
#define INS_REX_MODRM 1<<0
#define INS_REX_INDEX 1<<1
#define INS_REX_REG 1<<2
#define INS_REX_WIDE 1<<3

#define INS_MODRM_RM(v) (v&0x7)
#define INS_MODRM_REG(v) ((v>>3)&0x7)
#define INS_MODRM_MOD(v) (v>>6)