#pragma once
#include <stdint.h>

#define DT_MAGIC 0xD00DFEED3
#define DT_NODE_BEGIN 0x1
#define DT_NODE_END 0x2
#define DT_PROP 0x3
#define DT_NOP 0x4
#define DT_END 0x9

#define DT_MEMORY_NODE_STR "memory"
#define DT_CPU_NODE_STR "cpus"

#define DT_INC_NODE(node) ((uint32_t*)((uint64_t)node+sizeof(uint32_t)))
#define DT_STR_OFFSET(base,off) ((char*)((uint64_t)base+off))

struct fdtHeader{
    uint32_t magic;
    uint32_t totalSize;
    uint32_t structOff;
    uint32_t stringOff;
    uint32_t rsvmapOff;
    uint32_t version;
    uint32_t lastCompatVersion;
    uint32_t bootcpuid;
    uint32_t stringSize;
    uint32_t structSize;
}__attribute__((packed)) __scalar_storage_order__(("big-endian"));

struct fdtProp{
    uint32_t len;
    uint32_t nameOff;
}__attribute__((packed)) __scalar_storage_order__(("big-endian"));