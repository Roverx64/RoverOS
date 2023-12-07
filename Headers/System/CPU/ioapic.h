#pragma once

#include <stdint.h>

#define IOAPIC_ID 0x0
#define IOAPIC_VER 0x1
#define IOAPIC_ARB 0x2
#define IOAPIC_REDTBL 0x10
#define IOAPIC_REGSEL 0x0
#define IOAPIC_WIN 0x10
#define IOAPIC_REDTBL_ENTRY(i) (0x10+(2*i))

#define IOAPIC_REDTBL_APICID(id) ((uint64)id<<56)
#define IOAPIC_REDTBL_MASK 1<<16
#define IOAPIC_REDTBL_TRIGGER_LEVEL 1<<15
#define IOAPIC_REDTBL_REMOTE_IRR 1<<14
#define IOAPIC_REDTBL_POLARITY_LOW 1<<13
#define IOAPIC_REDTBL_DELIVERY_STATUS 1<<12
#define IOPAIC_REDTBL_DEST_LOGICAL 1<<11
#define IOAPIC_REDTBL_DELIVER_MODE(v) ((uint64)v<<10)
#define IOAPIC_DELIVERY_MODE_FIXED 0x0
#define IOAPIC_DELIVERY_MODE_LOWEST 0x1
#define IOAPIC_DELIVERY_MODE_SMI 0x2
#define IOAPIC_DELIVERY_MODE_NMI 0x4
#define IOAPIC_DELIVERY_MODE_INIT 0x5
#define IOAPIC_DELIVERY_MODE_EXTINIT 0x6
#define IOAPIC_REDTBL_VECTOR(v) ((uint64)v<<0)

extern void writeIOAPICInterrupt(uint8 source, uint8 vector, uint8 mode, bool edge, bool logical, uint8 dest);