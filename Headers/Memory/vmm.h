#pragma once
#include <stdint.h>

typedef struct{
    uint64_t *pml4e;
}pageSpace;

enum vType{
    VTYPE_FREE = 0, //Free for any purpose
    VTYPE_HEAP, //Heap region
    VTYPE_CODE, //Process' code
    VTYPE_DATA, //Process' data
    VTYPE_STACK, //Process' stack
    VTYPE_OTHER, //Misc stuff
    VTYPE_USER, //User allocated region
    VTYPE_LCODE, //Library code
    VTYPE_LDATA, //Library data
    VTYPE_VIMEM, //Video memory for UI based applications
    VTYPE_MMIO, //Memory mapped IO
    VTYPE_DEV, //Device ipc
    VTYPE_FILE, //File system data
    VTYPE_MAX_TYPE, //Maximum procmap type
    VTYPE_BLAME_NONE //No procmap was found
};

#define VFLAG_WRITE 1<<0
#define VFLAG_EXEC 1<<1
#define VFLAG_USER 1<<2
#define VFLAG_UC 1<<3
#define VFLAG_MAKE 1<<4
#define VFLAG_WT 1<<5
#define VFLAG_KERNEL 1<<6 //Allocate in kernel's virtual memory region
#define VFLAG_WC 1<<7

typedef struct{
    uint64_t base;
    uint32_t pages;
    uint32_t flags;
    uint8_t type; //I.E (v)Type of data stored here
    void *next;
}vMarker;

#define VMARKER_END(vm) (vm->base+(vm->pages*PAGE_SZ))

extern uint64_t virtToPhys(uint64_t virt);
extern void dumpVMarker(vMarker *head);
extern void *vmalloc(size_t bytes, uint8_t type, uint32_t flags);
extern void *vmallocPhys(uint64_t phys, size_t bytes, uint8_t type,  uint32_t flags);
extern void vmFree(void *virt);