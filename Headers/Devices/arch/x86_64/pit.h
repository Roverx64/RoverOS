#pragma once
#include <stdint.h>

#define PIT_PORT_CHANNEL0 0x40
#define PIT_PORT_CHANNEL1 0x41
#define PIT_PORT_CHANNEL2 0x42
#define PIT_PORT_MODE 0x43

#define PIT_MODE_BCD 1<<0
#define PIT_MODE_OPERATING(o) (((uint32_t)o&0x07)<<1)
#define PIT_MODE_ACCESS(a) (((uint32_t)a&0x03)<<4)
#define PIT_MODE_CHANNEL(c) (((uint32_t)c&0x03)<<6)

#define PIT_OPERATING_MODE_INT_ON_COUNT 0x0
#define PIT_OPERATING_MODE_ONE_SHOT 0x1
#define PIT_OPERATING_MODE_RATE 0x2
#define PIT_OPERATING_MODE_SQUARE 0x3
#define PIT_OPERATING_MODE_SOFT_STROBE 0x4
#define PIT_OPERATING_MODE_HARD_STROBE 0x5
#define PIT_OPERATING_MODE_RATE2 0x6 //Same as RATE
#define PIT_OPERATING_MODE_SQUARE2 0x7 //Same as SQUARE

#define PIT_ACCESS_LATCH 0x0
#define PIT_ACCESS_LO 0x1
#define PIT_ACCESS_HI 0x2
#define PIT_ACCESS_HILO 0x3

#define PIT_CHANNEL0 0x0
#define PIT_CHANNEL1 0x1
#define PIT_CHANNEL2 0x2
#define PIT_READBACK 0x3

extern void pitOneShot(uint16_t ticks);
extern void pitCalibrate(uint16_t ticks);