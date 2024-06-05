#pragma once

#include <stdint.h>
#include "ioapic.h"
#include <hpet.h>
#include <lapic.h>

#define RSDP_SIGNATURE 0x2052545020445352
#define XSDT_SIGNATURE 0x0

struct rsdp{
    uint64_t signature;
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t rsdt;
    uint32_t length;
    uint64_t xsdt;
    uint8_t extendedChecksum;
    uint8_t reserved[3];
}__attribute__((packed));

struct rsdt{
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint64_t oemTableid;
    uint32_t oemRevision;
    uint8_t creatorid[4];
    uint32_t creatorRevision;
}__attribute__((packed));

struct acpiDescHeader{
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint64_t oemTableid;
    uint32_t oemRevision;
    uint8_t creatorid[4];
    uint32_t creatorRevision;
}__attribute__((packed));

//Tables

#define FACP_SIGNATURE 0x50434146
#define APIC_SIGNATURE 0x43495041
#define WAET_SIGNATURE 0x54454157
#define BGRT_SIGNATURE 0x54524742

struct acpiFACP{
    uint32_t signature;
    uint32_t length;
    uint8_t majorVersion;
    uint8_t checksum;
    uint8_t oemid[6];
    uint64_t oemTableid;
    uint32_t oemRevision;
    uint8_t creatorid[4];
    uint32_t creatorRevision;
    uint32_t firmwareCTRL;
    //Incomplete
}__attribute__((packed));

//MADT/APIC

#define PCAT_COMPAT 1<<0
#define ICS_LAPIC 0
#define ICS_IOAPIC 1
#define ICS_ISO 2
#define ICS_NMI 3
#define ICS_LAPIC_NMI 4
#define ICS_LAPIC_AO 5
#define ICS_SOAPIC 6
#define ICS_LSAPIC 7
#define ICS_PLATFORM_IS 8
#define ICS_X2APIC 9
#define ICS_GICC 10
#define ICS_GICD 11
#define ICS_GICMSI 12
#define ICS_GICR 13
#define ICS_GICITS 14
#define ICS_MP_WAKEUP 15
#define ICS_CPIC 16
#define ICS_LPIC 17
#define ICS_HPIC 18
#define ICS_EPIC 19
#define ICS_MPIC 20
#define ICS_BPIC 21
#define ICS_LWPIC 22

struct acpiMADT{
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint64_t oemTableid;
    uint32_t oemRevision;
    uint8_t creatorID[4];
    uint32_t creatorRevision;
    uint32_t LocalInterruptControllerAddress;
    uint32_t flags;
}__attribute__((packed));

struct icsHeader{
    uint8_t type;
    uint8_t length;
}__attribute__((packed));

struct mpWake{
    uint8_t type;
    uint32_t length;
    uint16_t mailboxVersion;
    uint32_t reserved;
    uint64_t mailboxAddress;
}__attribute__((packed));

struct mpMailbox{
    uint16_t command;
    uint16_t reserved;
    uint32_t apicID;
    uint64_t wakeupVector;
}__attribute__((packed));

#define LAPIC_ENABLED 1<<0
#define LAPIC_ONLINE_CAPABLE 1<<1

struct lapic{
    struct icsHeader header;
    uint8_t uid;
    uint8_t lapicID;
    uint32_t flags;
}__attribute__((packed));

struct ioApic{
    struct icsHeader header;
    uint8_t id;
    uint8_t reserved;
    uint32_t ioAddr;
    uint32_t sib;
}__attribute__((packed));

struct interruptSourceOverride{
    struct icsHeader header;
    uint8_t bus;
    uint8_t source;
    uint32_t gsi;
    uint16_t flags;
}__attribute__((packed));

struct lapicNMI{
    struct icsHeader header;
    uint8_t uid;
    uint16_t flags;
    uint8_t lint;
}__attribute__((packed));