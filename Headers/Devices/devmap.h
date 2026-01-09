#pragma once

#include <stdint.h>

#define DEVMAP_RAMDISK 1<<0 //Marks driver as being on the ramdisk
#define DEVMAP_LOAD 1<<1 //Marks a driver to be loaded

#define DEVMAP_PCI 'P'
#define DEVMAP_GENERIC 'G'
#define DEVMAP_ALWAYS 'X'
#define DEVMAP_REQUEST 'O'

struct devmapEntry{
    uint16_t flags; //Driver flags
    uint16_t len; //Length of file name after this struct
    uint16_t deplen; //Size of u64 id array after this struct
    uint32_t id; //Connection specific id code
}__attribute__((packed));
