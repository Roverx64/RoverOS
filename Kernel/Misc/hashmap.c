#include <stdint.h>
#include <hashmap.h>
#include <string.h>
#include <kmalloc.h>

uint64_t hashNum(uint64_t n){
    return n;
}

uint64_t hashString(const char *str){
    uint64_t hash = 0;
    for(int i = 0; str[i] != '\0';++i){
        hash += str[i]^str[i+1];
    }
    return hash;
}

void hashmapInsert(hashmap *map, void *key, void *value, bool str){
    uint64_t hash = 0x0;
    uint64_t (*charFnc)(char*);
    uint64_t (*numFnc)(uint64_t*);
    if(str){
        charFnc = map->strHash;
        hash = charFnc((char*)key);
    }
    else{
        numFnc = map->numHash;
        hash = numFnc((uint64_t*)key);
    }
    uint64_t idx = hash%map->sz;
    if(map->keys[idx].hash != 0x0){
        hashkey *kv = (hashkey*)kmalloc(sizeof(hashkey));
        kv->hash = hash;
        kv->data = value;
        kv->chain = map->keys[idx].chain;
        map->keys[idx].chain = &kv;
        return;
    }
    map->keys[idx].hash = hash;
    map->keys[idx].data = value;
}

void hashmapRemove(hashmap *map, void *key, bool str){
    uint64_t hash = 0x0;
    uint64_t (*charFnc)(char*);
    uint64_t (*numFnc)(uint64_t*);
    if(str){
        charFnc = map->strHash;
        hash = charFnc((char*)key);
    }
    else{
        numFnc = map->numHash;
        hash = numFnc((uint64_t*)key);
    }
    uint64_t idx = hash%map->sz;
    if(map->keys[idx].chain == NULL){
        map->keys[idx].hash = 0x0;
        map->keys[idx].data = NULL;
        return;
    }
    hashkey *kv = (hashkey*)map->keys[idx].chain;
    //Fix chain
    memcpy(&map->keys[idx],kv,sizeof(hashkey));
    kfree(kv);
}

void *hashmapGetValue(hashmap *map, void *key, bool str){
    uint64_t hash = 0x0;
    uint64_t (*charFnc)(char*);
    uint64_t (*numFnc)(uint64_t*);
    if(str){
        charFnc = map->strHash;
        hash = charFnc((char*)key);
    }
    else{
        numFnc = map->numHash;
        hash = numFnc((uint64_t*)key);
    }
    uint64_t idx = hash%map->sz;
    hashkey *kv = &map->keys[idx];
    for(;;){
        if(kv->hash == hash){return kv->data;}
        if(kv->chain == NULL){return NULL;}
        kv = (hashkey*)kv->chain;
    }
    return NULL;
}


void hashmapResize(hashmap *map, size_t sz){
}

hashmap *hashmapCreate(size_t sz){
    hashmap *map = (hashmap*)kmalloc(sizeof(hashmap));
    map->keys = (hashkey*)kmalloc(sizeof(hashkey)*sz);
    memset(map->keys,0x0,sizeof(hashkey)*sz);
    map->sz = sz;
    map->numHash = &hashNum;
    map->strHash = &hashString;
    return map;
}

//h=(k*2654435761)%0x1<<63 (2^63)

//h=(0xafe2435634453*2654435761)%0x1
