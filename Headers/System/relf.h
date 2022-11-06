#pragma once

#include "types.h"

typedef uint64 ELF64_Addr;
typedef uint16 ELF64_Half;
typedef uint64 ELF64_Off;
typedef uint32 ELF64_Word;
typedef signed int ELF64_Sword;
typedef uint64 ELF64_Xword;
typedef signed long ELF64_Sxword;
typedef uint8 ELF64_Byte;
typedef uint16 ELF64_Section;

#define ELF64_MAGIC 0x7F454C46

struct ELF64eident{
    ELF64_Word magic;
    ELF64_Byte class;
    ELF64_Byte data;
    ELF64_Byte version;
    ELF64_Xword pad0;
    ELF64_Xword pad1;
}__attribute__((packed));

struct ELF64Header{
    struct ELF64eident ident;
    ELF64_Half type;
    ELF64_Half machine;
    ELF64_Word version;
    ELF64_Addr entry;
    ELF64_Off phoff;
    ELF64_Off shoff;
    ELF64_Word flags;
    ELF64_Half ehsize;
    ELF64_Half phentsize;
    ELF64_Half phnum;
    ELF64_Half shentsize;
    ELF64_Half shnum;
    ELF64_Half shstrndx;
}__attribute__((packed));

struct ELF64Section{
    ELF64_Word name;
    ELF64_Word type;
    ELF64_Xword flags;
    ELF64_Addr addr;
    ELF64_Off offset;
    ELF64_Xword size;
    ELF64_Word link;
    ELF64_Word info;
    ELF64_Xword align;
    ELF64_Xword entsize;
}__attribute__((packed));