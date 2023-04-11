#pragma once

#include <stdint.h>

#define RSDP_SIGNATURE 0x2052545020445352
#define XSDT_SIGNATURE 0x0

struct rsdp{
    uint64 signature;
    uint8 checksum;
    uint8 oemid[6];
    uint8 revision;
    uint32 rsdt;
    uint32 length;
    uint64 xsdt;
    uint8 extendedChecksum;
    uint8 reserved[3];
}__attribute__((packed));

struct rsdt{
    uint32 signature;
    uint32 length;
    uint8 revision;
    uint8 checksum;
    uint8 oemid[6];
    uint64 oemTableid;
    uint32 oemRevision;
    uint8 creatorid[4];
    uint32 creatorRevision;
}__attribute__((packed));

struct acpiDescHeader{
    uint32 signature;
    uint32 length;
    uint8 revision;
    uint8 checksum;
    uint8 oemid[6];
    uint64 oemTableid;
    uint32 oemRevision;
    uint8 creatorid[4];
    uint32 creatorRevision;
}__attribute__((packed));

//Tables

#define FACP_SIGNATURE 0x50434146
#define APIC_SIGNATURE 0x43495041
#define HPET_SIGNATURE 0x54455048
#define WAET_SIGNATURE 0x54454157
#define BGRT_SIGNATURE 0x54524742

struct acpiHPET{
    uint32 signature;
    uint32 length;
    uint8 revision;
    uint8 checksum;
    uint8 oemid[6];
    uint64 oemTableid;
    uint32 oemRevision;
    uint8 creatorid[4];
    uint32 creatorRevision;
    uint32 eventTimerBlockID;
    uint32 lowerBase0;
    uint32 lowerBase1;
    uint8 hpetNum;
    uint16 minClockTick;
    uint8 pgAttribute;
}__attribute__((packed));

struct acpiFACP{
    uint32 signature;
    uint32 length;
    uint8 majorVersion;
    uint8 checksum;
    uint8 oemid[6];
    uint64 oemTableid;
    uint32 oemRevision;
    uint8 creatorid[4];
    uint32 creatorRevision;
    uint32 firmwareCTRL;
    //Incomplete
}__attribute__((packed));