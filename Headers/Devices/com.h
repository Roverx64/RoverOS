#pragma once

#include <stdint.h>

#define COM1 0x3F8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

//Kernel debug log port
#define KDEBUG_PORT COM1

#define DLV_DIVISOR 115200

#define OFFSET_DLV_HIGH 0x0
#define OFFSET_DLV_LOW 0x1
#define OFFSET_FIFO 0x2
#define OFFSET_LCR 0x3
#define OFFSET_MCR 0x4
#define OFFSET_LSR 0x5

extern bool initCOMPort(uint16_t port);
extern void writeByte(uint16_t port, uint8_t byte);
extern void writeString(uint16_t port, const char *str);