#pragma once

#include <stdint.h>

#define ACPI_SIGNATURE 0x2052545020445352

struct rdsp{
    uint64 signature;
    uint8 checksum;
    uint8 oemid[6];
    uint8 revision;
    uint32 rsdtAddr;
    uint32 length;
    uint64 xsdt;
    uint8 extendedChecksum;
    uint8 reserved[3];
}__attribute__((packed));