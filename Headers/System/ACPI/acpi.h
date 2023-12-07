#pragma once

#include <stdint.h>
#include "ioapic.h"

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
    uint32 upper;
    uint64 bar;
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

//MADT/APIC

#define PCAT_COMPAT 1<<0
#define LAPIC 0
#define IOAPIC 1
#define ISO 2
#define NMI 3
#define LAPIC_NMI 4
#define LAPIC_AO 5
#define SOAPIC 6
#define LSAPIC 7
#define PLATFORM_IS 8
#define X2APIC 9
#define GICC 10
#define GICD 11
#define GICMSI 12
#define GICR 13
#define GICITS 14
#define MP_WAKEUP 15
#define CPIC 16
#define LPIC 17
#define HPIC 18
#define EPIC 19
#define MPIC 20
#define BPIC 21
#define LWPIC 22

struct acpiMADT{
    uint32 signature;
    uint32 length;
    uint8 revision;
    uint8 checksum;
    uint8 oemid[6];
    uint64 oemTableid;
    uint32 oemRevision;
    uint8 creatorID[4];
    uint32 creatorRevision;
    uint32 LocalInterruptControllerAddress;
    uint32 flags;
}__attribute__((packed));

struct icsHeader{
    uint8 type;
    uint8 length;
}__attribute__((packed));

struct mpWake{
    uint8 type;
    uint32 length;
    uint16 mailboxVersion;
    uint32 reserved;
    uint64 mailboxAddress;
}__attribute__((packed));

struct mpMailbox{
    uint16 command;
    uint16 reserved;
    uint32 apicID;
    uint64 wakeupVector;
}__attribute__((packed));

#define LAPIC_ENABLED 1<<0
#define LAPIC_ONLINE_CAPABLE 1<<1

struct lapic{
    struct icsHeader header;
    uint8 uid;
    uint8 lapicID;
    uint32 flags;
}__attribute__((packed));

struct ioApic{
    struct icsHeader header;
    uint8 id;
    uint8 reserved;
    uint32 ioAddr;
    uint32 sib;
}__attribute__((packed));

struct interruptSourceOverride{
    struct icsHeader header;
    uint8 bus;
    uint8 source;
    uint32 gsi;
    uint16 flags;
}__attribute__((packed));

struct lapicNMI{
    struct icsHeader header;
    uint8 uid;
    uint16 flags;
    uint8 lint;
}__attribute__((packed));