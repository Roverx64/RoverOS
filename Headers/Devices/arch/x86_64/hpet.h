#pragma once

#include <stdint.h>

struct acpiHPET{
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oemid[6];
    uint64_t oemTableid;
    uint32_t oemRevision;
    uint8_t creatorid[4];
    uint32_t creatorRevision;
    uint32_t eventTimerBlockID;
    uint32_t upper;
    uint64_t bar;
    uint8_t hpetNum;
    uint16_t minClockTick;
    uint8_t pageProtection;
}__attribute__((packed));

struct hpetTimerMMIO{
    uint64_t configCapability;
    uint64_t compareValue;
    uint64_t interruptRoute;
    uint64_t pad0; //Padding for next timer
}__attribute__((packed));

struct hpetMMIO{
    uint64_t capabilities;
    uint64_t pad0; //Padding
    uint64_t configuration;
    uint64_t pad1; //Padding
    uint64_t interruptStatus;
    uint64_t pad2; //Padding
    uint64_t mainCounter;
    uint64_t pad3; //Padding
    volatile struct hpetTimerMMIO timer[32]; //Minimum 3 timers are required
}__attribute__((packed));

//For device tree
struct hpetInfo{
    volatile struct hpetMMIO *mmio;
    uint64_t physBAR; //Physical address of the BAR
    uint8_t timers; //Timers present
    bool extended; //True if 64-bit timers are allowed
    bool legacyRoute; //True if HPET allows legacy routing
    uint32_t minClock;
    uint32_t clockPeriod; //In femto seconds
    uint8_t pageProtection;
};

#define HPET_SIGNATURE 0x54455048 //ACPI signature

#define HPET_GENERAL_CAPABILITIES 0x0
#define HPET_GENERAL_CAPABILITIES_REV(cap) (cap&0xFF)
#define HPET_GENERAL_CAPABILITIES_TIMERS(cap) ((cap>>8)&0x0F)
#define HPET_GENERAL_CAPABILITIES_COUNT_SIZE(cap) ((cap>>13)&0x1)
#define HPET_GENERAL_CAPABILITIES_LEG_ROUTE(cap) ((cap>>15)&0x1)
#define HPET_GENERAL_CAPABILITIES_VENDOR_ID(cap) ((cap>>16)&0xFFFF)
#define HPET_GENERAL_CAPABILITIES_CLOCK_PER(cap) (cap>>32)

#define HPET_GENERAL_CONFIGURATIION 0x10
#define HPET_GENERAL_CONFIGURATION_ENABLE_CNF 1<<0
#define HPET_GENERAL_CONFIGURATION_LEG_RT_CNF 1<<1

#define HPET_GENERAL_INTERRUPT_STATUS 0x20
#define HPET_GENERAL_INTERRUPT_STATUS_CHECK(status,timer) ((status>>timer)&0x1)

#define HPET_MAIN_COUNTER_VAL 0xF0

#define HPET_TIMER_BASE_OFFSET(timer) (0x100+(timer*0x20))
#define HPET_TIMER_CONFIG_OFFSET 0x0
#define HPET_TIMER_COMPAR_OFFSET 0x8
#define HPET_TIMER_FSB_INT_OFFSET 0x10

#define HPET_TIMER_INT_TYPE_CNF 1<<1
#define HPET_TIMER_INT_ENB_CNF 1<<2
#define HPET_TIMER_TYPE_CNF 1<<3
#define HPET_TIMER_PER_INT_CAP 1<<4
#define HPET_TIMER_SIZE_CAP 1<<5
#define HPET_TIMER_VAL_SET_CNF 1<<6
#define HPET_TIMER_32MODE_CNF 1<<8
#define HPET_TIMER_INT_ROUTE_CNF(cfg,v) (((uint64)v<<9)|cfg)
#define HPET_TIMER_FSB_EN_CNF 1<<14
#define HPET_TIMER_FSB_INT_DEL_CAP 1<<15
#define HPET_TIMER_INT_ROUTE_CAP(cfg) (cfg>>32)

#define HPET_FSB_INT_ADDR(cnf,v) (cnf|(v&0xFFFFFFFF))
#define HPET_FSB_INT_VAL(cnf,v) (cnf|(v<<32))
