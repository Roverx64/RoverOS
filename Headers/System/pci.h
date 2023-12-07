#pragma once
#include <stdint.h>

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC
#define PCI_HEADER_VENDOR 0x0
#define PCI_HEADER_CLASS 0x8
#define PCI_HEADER_SUBCLASS 0x2C
#define PCI_HEADER_DEVICEID 0x0

#define PCI_VENDOR_INTEL 0x8086

extern void writePCIAddress(uint8 bus, uint8 dev, uint8 fn, uint8 reg);
extern uint32 readPCI32(uint16 addr);