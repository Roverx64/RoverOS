#include <stdint.h>
#include <string.h>
#include "ramdisk.h"
#include "kprint.h"
#include "kernel.h"
#include "paging.h"
#include "pmm.h"
#include "kmalloc.h"
#include "elf.h"
#include "unit.h"
#include "hashtable.h"

//This file is used to load kernel libraries to memory
//Also keeps track of loaded libraries/drivers

hashtable *tbl;
mutex_t liblock;

//Loads a dynamic library (.lib) to memory
//For loading libraries from ramdisk
/*
bool loadRDLibrary(RDFL *lib, uint16_t flags){
    kinfo("Loading %s\n",rdName(lib));
    ELF64Header header;
    //rdRead(lib,&header,sizeof(header));
    if(header.ident.fmagic != ELF_FMAGIC){
        kerror("Invalid ELF magic\n");
        return false;
    }
    //Check endianess and bit size
    if(header.ident.class != ELF_CLASS64){
        kerror("ELF is not 64-bit");
        return false;
    }
    //TODO: check endianess
    struct unitLib *library = (struct unitLib*)kmalloc(sizeof(struct unitLibrary));
    KASSERT((library != NULL),"Failed to create library tracker");
    //Check sections needed for mmap
    uint16_t sections = 0;
    for(ELF64_Off i = 0;i < header.phnum;){
    }
    return false;
}

//Loads a file from the VFS
bool loadLibrary(){
    return false;
}

bool isLibLoaded(char *lib){
    GET_LOCK(liblock);
    struct unitLibrary *l;
    if(!hashtableFetch(tbl,lib,strlen(lib),(void**)&l)){FREE_LOCK(liblock); return false;}
    FREE_LOCK(liblock);
    return true;
}

bool referenceLib(char *lib){
    struct unitLibrary *l;
    GET_LOCK(liblock);
    if(!hashtableFetch(tbl,lib,strlen(lib),(void**)&l)){FREE_LOCK(liblock); return false;}
    l->references += 1;
    FREE_LOCK(liblock);
    return true;
}

//Loads primary libraries and boot drivers from ramdisk
void initUnits(){

}

*/