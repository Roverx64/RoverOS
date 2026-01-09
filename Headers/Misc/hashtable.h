#pragma once

#include <stdint.h>
#include <stdbool.h>

#define HASHTABLE_FLAG_COPY 1<<0 //Allocate and copy the hash

//#define HASHTABLE_DEBUG 1

typedef struct{
    void* hash; //Hash is NULL when key is unused
    size_t hashLen;
    uint8_t flags;
    uint64_t data; //Pointer to data, or just a plain u64 number
    void *chain; //Points to next hash bucket if applicable
}hashbucket;

typedef struct{
    hashbucket *buckets;
    uint16_t bucketCount;
}hashtable;

extern uint64_t hashData(void *hash, size_t bytes, uint16_t buckets);
extern hashtable *hashtableCreate(uint16_t buckets);
extern void hashtableDestroy(hashtable *table);
extern void hashtableInsert(hashtable *table, uint64_t data, void* hash, size_t bytes, uint8_t flags, bool dyn);
extern void hashtableRemove(hashtable *table, void *hash, size_t bytes);
extern bool hashtableFetch(hashtable *table, void *hash, size_t bytes, void *data);
extern bool hashtableUpdate(hashtable *table, uint64_t data, void *hash, size_t bytes, uint8_t flags);
