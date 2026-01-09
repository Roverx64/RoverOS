#include <stdint.h>
#include <stdbool.h>
#include <hashtable.h>
#include <string.h>
#include <kmalloc.h>
#include <kernel.h>
#include <kprint.h>

/*!
    !D Hashes 'hash'
    !I hash: Data to hash
    !I bytes: Number of bytes in 'hash'
    !I buckets: Number of buckets in the table
    !R 64bit hash
    !C NOTE: Hash code is djb2 hashing modified from 'http://www.cse.yorku.ca/~oz/hash.html'
*/
uint64_t hashData(void *hash, size_t bytes, uint16_t buckets){
    size_t n = 0;
    uint8_t *h = (uint8_t*)hash;
    for(size_t i = 0; i < bytes; ++i){
        n = (n<<5)+n+(h[i]);
    }
    #ifdef HASHTABLE_DEBUG
    kinfo("Hashed data to 0x%x\n",(uint64_t)n%buckets);
    #endif
    return n%buckets;
}

/*!
    !D Creates a hashtable
    !I bucktets: Number of buckets to create
    !R hashtable on success or NULL on failure
*/
hashtable *hashtableCreate(uint16_t buckets){
    hashtable *table = (hashtable*)kmalloc(sizeof(hashtable));
    KASSERT((table != NULL),"NULL table");
    table->buckets = (hashbucket*)kmalloc(sizeof(hashbucket)*buckets);
    KASSERT((table->buckets != NULL),"NULL keys");
    table->bucketCount = buckets;
    return table;
}

/*!
    !D Resizes a hashtable
    !C NOTE: Write the code bud!
*/
void hashtableResize(uint16_t newBuckets){

}

/*!
    !D Frees a hashbucket
    !I bucket: bucket to free
    !R None
*/
void freeKey(hashbucket *bucket){
    if(bucket->flags&HASHTABLE_FLAG_COPY){kfree(bucket->hash);}
    if(!bucket->chain){return;}
    bucket = (hashbucket*)bucket->chain;
    while(true){
        hashbucket *n = (hashbucket*)bucket->chain;
        if(bucket->flags&HASHTABLE_FLAG_COPY){kfree(bucket->hash);}
        kfree(bucket);
        bucket = n;
        if(n == NULL){return;}
    }
}

/*!
    !D Destroys a hashtable
    !I table: hashtable to destroy
    !R None
*/
void hashtableDestroy(hashtable *table){
    if(!table){return;}
    if(!table->buckets){kfree(table); return;}
    //Loop through keys and free them
    for(uint16_t i = 0; i < table->bucketCount; ++i){
        freeKey(&table->buckets[i]);
    }
    kfree(table->buckets);
    kfree(table);
}

/*!
    !D Inserts a new entry into a hashtable
    !I table: hashtable to use
    !I data: Pointer to data
    !I hash: Data to hash
    !I bytes: Number of bytes in 'hash'
    !I flags: hashtable flags OR'd together
    !I dyn: Unused
    !R None
*/
void hashtableInsert(hashtable *table, uint64_t data, void* hash, size_t bytes, uint8_t flags, bool dyn){
    //Hash data
    uint64_t ent = hashData(hash,bytes,table->bucketCount);
    KASSERT((ent < table->bucketCount),"Invalid hash");
    //Copy hash if needed
    void *thsh = hash;
    if(flags&HASHTABLE_FLAG_COPY){
        thsh = (void*)kmalloc(sizeof(uint8_t)*bytes);
        memcpy(thsh,hash,bytes);
    }
    //Insert the hash
    if(table->buckets[ent].hash == NULL){
        table->buckets[ent].hash = thsh;
        table->buckets[ent].data = data;
        table->buckets[ent].chain = NULL;
        table->buckets[ent].flags = flags;
        table->buckets[ent].hashLen = bytes;
        #ifdef HASHTABLE_DEBUG
        kinfo("Successfully added hash 0x%lx to table\n",ent);
        #endif
        return;
    }
    //Allocate and add a new chain
    hashbucket *chainStart = &table->buckets[ent];
    hashbucket *bucket = NULL;
    if(chainStart->chain == NULL){
        chainStart->chain = (void*)kmalloc(sizeof(hashbucket));
        bucket = (hashbucket*)chainStart->chain;
        goto next;
    }
    //Follow chain to next free slot
    while(chainStart->chain != NULL){
        if(chainStart->hash == NULL){
            bucket = chainStart;
            goto next;
        }
        chainStart = (hashbucket*)chainStart->chain;
    }
    //Allocate new bucket
    chainStart->chain = (void*)kmalloc(sizeof(hashbucket));
    bucket = (hashbucket*)chainStart->chain;
    //Init bucket
    next:
    bucket->hash = thsh;
    bucket->data = data;
    bucket->chain = NULL;
    bucket->flags = flags;
    bucket->hashLen = bytes;
    #ifdef HASHTABLE_DEBUG
    kinfo("Added hash 0x%lx to bucket chain\n",ent);
    #endif
}

/*!
    !D Returns a hashbucket a hash matching 'hash'
    !I table: hashtable to use
    !I hash: Data to hash
    !I bytes: Number of bytes in 'hash'
    !I prev: Pointer to a pointer variable
    !R hashbucket on success and sets 'prev' if hashbucket is part of a chain, NULL otherwise.
    !R NULL on error
*/
hashbucket *findKey(hashtable *table, void *hash, size_t bytes, hashbucket **prev){
    uint64_t ent = hashData(hash,bytes,table->bucketCount);
    if(table->buckets[ent].hash == NULL){return NULL;}
    KASSERT((ent < table->bucketCount),"Invalid hash");
    if(prev){*prev = NULL;}
    if(table->buckets[ent].chain == NULL){
        return &table->buckets[ent];
    }
    //Loop through keys and find correct key
    hashbucket *test = (hashbucket*)table->buckets[ent].chain;
    hashbucket *key = NULL;
    #ifdef HASHTABLE_DEBUG
    kinfo("Testing chain\n");
    if(test == NULL){kinfo("NULL chain\n");}
    #endif
    while(test){
        if(test->hashLen != bytes){goto nxt;}
        #ifdef HASHTABLE_DEBUG
        kinfo("Checking chain entry\n");
        #endif
        //Compare memory to check
        for(uint32_t i = 0; i < bytes; ++i){
            if(((uint8_t*)hash)[i] != ((uint8_t*)test->hash)[i]){goto nxt;}
        }
        //Keys match
        key = test;
        break;
        nxt:
        test = (hashbucket*)test->chain;
    }
    return key;
}

/*!
    !D Removes a hash entry from a table
    !I table: hashtable to use
    !I hash: Data to hash
    !I bytes: Number of bytes in 'hash'
    !R None
*/
void hashtableRemove(hashtable *table, void *hash, size_t bytes){
    hashbucket *prev;
    hashbucket *key = findKey(table,hash,bytes,&prev);
    if(!key){return;}
    if(key->flags&HASHTABLE_FLAG_COPY){kfree(key->hash);}
    key->hash = NULL;
    key->data = 0;
    if(prev){prev->chain = key->chain; kfree(key);}
}

/*!
    !D Fetches data pointer from a hashtable entry
    !I table: hashtable to use
    !I hash: Data to hash
    !I bytes: Number of bytes in 'hash'
    !I data: Location to write data pointer to
    !R true on success, false otherwise
*/
bool hashtableFetch(hashtable *table, void *hash, size_t bytes, void *data){
    hashbucket *key = findKey(table,hash,bytes,NULL);
    #ifdef HASHTABLE_DEBUG
    kinfo("Fetch returned key 0x%lx\n",(uint64_t)key);
    #endif
    if(!key){return false;}
    *((uint64_t*)data) = key->data;
    return true;
}

/*!
    !D Updates a hashtable entry
    !I table: hashtable to use
    !I hash: Data to hash
    !I bytes: Number of bytes in 'hash'
    !I flags: Flags to write to entry
    !R true on success, false otherwise
*/
bool hashtableUpdate(hashtable *table, uint64_t data, void *hash, size_t bytes, uint8_t flags){
    hashbucket *key = findKey(table,hash,bytes,NULL);
    if(!key){return false;}
    //Update hash
    if(key->flags&HASHTABLE_FLAG_COPY){
        if(flags&HASHTABLE_FLAG_COPY){
            key->hash = krealloc(key->hash,sizeof(uint8_t)*bytes);
        }
        else{kfree(key->hash); key->hash = hash;}
    }
    else{
        if(flags&HASHTABLE_FLAG_COPY){
            key->hash = kmalloc(sizeof(uint8_t)*bytes);
            memcpy(key->hash,hash,bytes);
        }
    }
    //Update the rest
    key->hashLen = bytes;
    key->data = data;
    key->flags = flags;
    return true;
}