#pragma once

#include <stdint.h>

#define PAGE_SZ 0x1000

#define KALIGN(addr) ((uint64_t)addr&PAGE_ALIGN)

#define PAGE_WRITE 1<<0
#define PAGE_EXECUTE 1<<1

#define FLAG_PRESENT 1<<0
#define FLAG_PAGE 1<<1
#define FLAG_DIRTY 1<<2
#define FLAG_ACCESS 1<<10
#define FLAG_NX 1<<54

/*
//4KB translation 52-bit PA

Table
|Attr |IGN     |0|Base                                  |IG|OA|IGN   |11|
|00000|00000000|0|00000000000000000000000000000000000000|00|00|000000|11|

Block L0
|Attributes    |Base       |RES                   |T|RES |LA|OA|L Attr|01|
|00000000000000|00000000000|0000000000000000000000|0|0000|00|00|000000|01|

Block L1
|Attributes    |Base                |RES          |T|RES |LA|OA|L Attr|01|
|00000000000000|00000000000000000000|0000000000000|0|0000|00|00|000000|01|

Block L2
|Attributes    |Base                         |RES |T|RES |LA|OA|L Attr|01|
|00000000000000|00000000000000000000000000000|0000|0|0000|00|00|000000|01|

Page
|Attributes    |Base                                  |LA|OA|L Attr|11|
|00000000000000|00000000000000000000000000000000000000|00|00|000000|11|
*/