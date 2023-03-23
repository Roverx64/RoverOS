#pragma once

#include <stdint.h>

typedef uint64 ELF64_Addr;
typedef uint16 ELF64_Half;
typedef uint64 ELF64_Off;
typedef uint32 ELF64_Word;
typedef signed int ELF64_Sword;
typedef uint64 ELF64_Xword;
typedef signed long ELF64_Sxword;
typedef uint8 ELF64_Byte;
typedef uint16 ELF64_Section;

#define ELF_MAGIC "\177ELF"

typedef struct ELF64eident{
    ELF64_Byte magic[4];
    ELF64_Byte class;
    ELF64_Byte data;
    ELF64_Byte version;
    ELF64_Word pad0;
}ELF64ident;


typedef struct ELF64HeaderS{
    ELF64ident ident;
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
}ELF64Header;

typedef struct ELF64SectionS{
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
}ELF64Section;

typedef struct ELF64ProgramHeaderS{
    ELF64_Word type;
    ELF64_Word flags;
    ELF64_Off offset;
    ELF64_Addr vaddr;
    ELF64_Addr paddr;
    ELF64_Xword filesz;
    ELF64_Xword memsz;
    ELF64_Xword align;
}ELF64ProgramHeader;

//Ident
#define ELF_CLASS32 1
#define ELF_CLASS64 2
//Section
#define SHT_NULL 0 //Inactive/Ignore
#define SHT_PROGBITS 1 //Program defined
#define SHT_SYMTAB 2 //Symbol table
#define SHT_STRTAB 3 //String table
#define SHT_RELA 4 //Explicit appends
#define SHT_HASH 5 //Symbol hash table
#define SHT_DYNAMIC 6 //Linking info
#define SHT_NOTE 7 //Comments
#define SHT_NOBITS 8 //BSS section
#define SHT_REL 9 //No explicit appends
#define SHT_SHLIB 10 //Ingore/Reserved
#define SHT_DYNSYM 11 //Dynamic linking
//Flags
#define SHF_WRITE 0x1 //Should be writable
#define SHF_ALLOC 0x2 //Occupies space in memory
#define SHF_EXEC 0x4 //Executable code
//Program header
#define PT_NULL 0 //Null/Ignore
#define PT_LOAD 1 //Loadable
#define PT_DYNAMIC 2 //Dynamic segment
#define PT_INTERP 3 //Path to interpreter
#define PT_NOTE 4 //Information
#define PT_SHLIB 5 //Reserved
#define PT_PHDR 6 //Program header segment
//Flags
#define PF_X 0x1 //Executable
#define PF_W 0x2 //Writable
#define PF_R 0x4 //Readable