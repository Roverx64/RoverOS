#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <cpu.h>

//Pointers to device info structures

struct systemInformation{
    void *tsc;
    void *ioapic;
    void *hpet;
    void *xhci;
    void *ahci;
    void *pci;
    uint16_t cpuCount;
    //struct cpuInfo *cpu;
};

extern struct systemInformation sysinf;