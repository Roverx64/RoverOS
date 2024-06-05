#pragma once

typedef struct{
    uint64_t hash;
    void *data; //Pointer to data
    void *chain; //Points to next hash key if applicable
}hashkey;

typedef struct{
    hashkey *keys;
    size_t sz;
    void *numHash;
    void *strHash;
}hashmap;

extern void hashmapInsert(hashmap *map, void *key, void *value, bool str);
extern void hashmapRemove(hashmap *map, void *key, bool str);
extern void *hashmapGetValue(hashmap *map, void *key, bool str);
extern void hashmapResize(hashmap *map, size_t sz);
extern hashmap *hashmapCreate(size_t sz);