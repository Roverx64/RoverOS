#pragma once

#include <stdint.h>
#include <mutex.h>

#define DEVICE_BUS_PCI 0
#define DEVICE_BUS_AHCI 1

struct deviceInfo{
    bool claimed; //Some process/driver owns this device already
};

struct deviceList{
    struct deviceInfo *devices;
    uint32_t listLen;
    uint32_t unclaimed;
};

//This structure is passed as pointer to all drivers
//Always mapped in mem as RO
//All devices are put in the 'unknown' category
//until moved to a specific listing
struct systemInfo{
    struct deviceList storageController;
    struct deviceList networkController;
    struct deviceList displayController;
    struct deviceList mmController;
    struct deviceList memoryController;
    struct deviceList serialController;
    struct deviceList wirelessController;
    struct deviceList unknown;
    //OS specific device lists
    struct deviceList storage; //set by AHCI,XHCI,etc
};