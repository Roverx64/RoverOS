#include <stdint.h>
#include <stdbool.h>
#include <vmm.h>
#include <hashtable.h>
#include <elf.h>
#include <string.h>
#include <kernel.h>
#include <kprint.h>
#include <dwarf.h>
#include <ksym.h>

ELF64Sym *symStart;
uint64_t symn = 0;
uint64_t strBase = 0;
ELF64Header *header = NULL;
ELF64Section *section = NULL;
uint64_t symbase = 0x0;
uint64_t secstrBase = 0x0;

ELF64Section *dbgInfo = NULL;
ELF64Section *dbgLine = NULL;
ELF64Section *dbgStr = NULL;

bool checkSection(char *secName, ELF64Section *sec){
    char *name = (char*)(secstrBase+sec->name);
    if(strcmp(secName,name) != 0x0){return false;}
    return true;
}

/*!
    !D Initilizes kernel debug symbols
    !I ksym: Pointer to kernel symbols in memory
    !R None
*/
void initKsym(void *ksym){
    kinfo("Initilizing ksymbols\n");
    //Enumerate number of symbols to bind
    symbase = (uint64_t)ksym;
    header = (ELF64Header*)ksym;
    section = (ELF64Section*)((uint64_t)ksym+header->shoff);
    ELF64Section *sym = NULL;
    ELF64Section *str = NULL;
    for(ELF64_Half i = 0; i < header->shnum; ++i){
        if(section[i].type == SHT_SYMTAB){kinfo("Sym at 0x%lx\n",section[i].offset); symn = section[i].size; sym = &section[i];}
        if(section[i].type == SHT_STRTAB){
            //The bigger string section is most likely for symbols
            if(!str){str = &section[i]; continue;}
            if(section[i].size < str->size){
                secstrBase = (uint64_t)((uint64_t)ksym+section[i].offset);
                continue;
            }
            secstrBase = (uint64_t)((uint64_t)ksym+str->offset);
            str = &section[i];
        }
    }
    KASSERT((sym != NULL),"No sym section");
    KASSERT((str != NULL),"No str section");
    kinfo("Str at 0x%lx\n",str->offset);
    symn = symn/sizeof(ELF64Sym);
    //Create and fill hashtable
    //ksym = hashtableCreate(symn);
    ELF64Sym *symh = (ELF64Sym*)((uint64_t)ksym+sym->offset);
    symStart = symh;
    strBase = (uint64_t)ksym+str->offset;
    char *fstr = NULL;
    //Fill hashtable
    char *name = "None\0";
    /*for(uint64_t i = 0; i < symn; ++i){
        if((symh[i].info&0x0F) == STT_NOTYPE){continue;}
        if((symh[i].info&0x0F) == STT_SECTION){continue;}
        fstr = (char*)(strBase+symh[i].name);
        if(symh[i].name == 0x0){
            fstr = name;
        }
        kinfo("Found sym [%s][ptr: 0x%lx][Sz: 0x%lx]\n",fstr,symh[i].value,(uint64_t)symh[i].size);
        kinfo("{0x%lx|0x%x|0x%x|0x%x|0x%lx|0x%lx}\n",(uint64_t)symh[i].name+str->offset,(uint32_t)symh[i].info,(uint32_t)symh[i].other,(uint32_t)symh[i].shndx,(uint64_t)symh[i].value,(uint64_t)symh[i].size);
        //hashtableInsert(ksym,(uint64_t)symh[i].name,&symh[i].value,sizeof(symh[i].value),0,false);
    }*/
    //Find debug sections]
    for(ELF64_Half i = 0; i < header->shnum; ++i){
        if(section[i].type != SHT_PROGBITS){continue;}
        if(checkSection(".debug_info",&section[i])){
            kinfo("Found .debug_info section\n");
            dbgInfo = &section[i];
            continue;
        }
        if(checkSection(".debug_line",&section[i])){
            kinfo("Found .debug_line section\n");
            dbgLine = &section[i];
            continue;
        }
        if(checkSection(".debug_str",&section[i])){
            kinfo("Found .debug_str section\n");
            dbgStr = &section[i];
            continue;
        }
    }
    kinfo("Loaded bindings\n");
}

/*!
    !D Returns a symbol from an address ptr
    !I ptr: address to search
    !R ELF Symbol or NULL on failure
*/
ELF64Sym *getSymbol(uint64_t ptr){
    if(ptr == 0x0){return NULL;}
    //Loop through symbols
    ELF64Sym *sym = symStart;
    for(uint64_t i = 0; i < symn; ++i){
        if((sym[i].info&0x0F) == STT_NOTYPE){continue;}
        if((sym[i].info&0x0F) == STT_SECTION){continue;}
        if((sym[i].value <= ptr) && ((sym[i].value+sym[i].size) >= ptr)){
            return &sym[i];
        }
        continue;
    }
    return NULL;
}

/*!
    !D Gets a symbols name
    !I sym: symbol to use
    !R Pointer to symbol name
*/
char *symbolToName(ELF64Sym *sym){
    return (char*)(sym->name+strBase);
}