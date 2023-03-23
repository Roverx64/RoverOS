#pragma once

//stdint but without causing issues for UEFI headers

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef unsigned long long uintptr;
typedef enum {false, true} bool;