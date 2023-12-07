#include <stdint.h>
#include <math.h>
#include "malloc.h"
#include "pmm.h"
#include "paging.h"

//Very early malloc
//Nowhere near the final version
//TODO: Redo most of this

heapBlock *findSuitableBlock(size sz, heapBlock *start){
    sz += sizeof(heapBlock);
    heapBlock *block = start; /*Get task's initial block*/
    for(;(block->flags&HEAP_FLAG_NEXT) != 0x0;){
        if(block->free >= sz){
            //Split
            heapBlock *newBlock = (heapBlock*)((uint64)block+sizeof(heapBlock)+block->allocated);
            //Setup new block
            newBlock->magic = 0x0; /*Task magic*/
            newBlock->allocated = sz-sizeof(heapBlock);
            newBlock->free = (block->free-sz);
            newBlock->offset = sizeof(newBlock)+block->allocated;
            newBlock->flags = 0x0;
            //Setup prev block
            block->flags |= HEAP_FLAG_NEXT;
            block->free = 0x0;
            //Setup next block
            heapBlock *nextBlock = (heapBlock*)((uint64)newBlock+sizeof(heapBlock)+newBlock->allocated+newBlock->free);
            nextBlock->offset = (uint64)((uint64)nextBlock-(sizeof(heapBlock)*2)-newBlock->free-newBlock->allocated);
            return newBlock;
        }
    }
    //Allocate new block
    block->flags |= HEAP_FLAG_NEXT;
    heapBlock *newBlock = (heapBlock*)((uint64)block+sizeof(heapBlock)+block->allocated);
    newBlock->magic = 0x0; /*Task magic*/
    newBlock->allocated = sz-sizeof(heapBlock);
    newBlock->free = (block->free-sizeof(newBlock)-newBlock->allocated);
    newBlock->offset = sizeof(newBlock)+block->allocated;
    newBlock->flags = 0x0;
    //Fix prev block
    block->flags |= HEAP_FLAG_NEXT;
    return newBlock;
}

void *malloc(size sz){
    //Get start block
    heapBlock *start = 0x0;
    if(start->magic != 0x0){/*Kill proccess for corrupted heap*/}
    heapBlock *block = findSuitableBlock(sz,start);
    return (void*)((uint64)block+sizeof(heapBlock));
}

void free(void *ptr){
    heapBlock *block = (heapBlock*)((uint64)ptr-sizeof(heapBlock));
    if(block->magic != 0x0){/*Kill proccess for invalid ptr*/}
    
}