#include <stdint.h>
#include <kmalloc.h>
#include <kprint.h>
#include <dstack.h>

/*!
  Data stack management utils
*/

/*!
    !D Shrinks data stack size
    !I stack: Data stack to act on
    !I sz: New size to use
    !C NOTE: Function resizes to stack top if new size will truncate data
    !C LOCK: This function aquires the stack's lock
*/
void shrinkStack(dstack *stack, size_t sz){
    GET_LOCK(stack->lock);
    size_t inUse = DSTACK_BYTES_IN_USE(stack);
    size_t shrinkSize = stack->maxSize;
    shrinkSize -= (inUse > sz) ? inUse : sz;
    void *nptr = krealloc(stack->base,shrinkSize); //!< This really should never fail in this scenario
    if(!nptr){kerror("Failed to realloc somehow???\n"); FREE_LOCK(stack->lock); return;}
    stack->base = nptr;
    stack->top = (void*)((uint64_t)nptr+inUse);
    stack->maxSize = shrinkSize;
    FREE_LOCK(stack->lock);
}

void growStack(dstack *stack, size_t sz){
    GET_LOCK(stack->lock);
    size_t inUse = (uint64_t)stack->top-(uint64_t)stack->base;
    void *nptr = krealloc(stack->base,stack->maxSize+sz);
    if(!nptr){kerror("Failed to allocate memory\n"); FREE_LOCK(stack->lock); return;}
    stack->base = nptr;
    stack->top = (void*)((uint64_t)nptr+inUse);
    stack->maxSize += sz;
    FREE_LOCK(stack->lock);
}

/*!
    !D Pops next item from the stack top
    !I stack: Data stack to act on
    !I fetched: pointer to bool to set
    !R Returns stack top data on success
    !C NOTE: Caller must check if 'fetched' is true, otherwise returned data is invalid
    !C LOCK: This function aquires the stack's lock
*/
uint64_t popStack(dstack *stack, bool *fetched){
    *fetched = false;
    GET_LOCK(stack->lock);
    if(stack->base == stack->top){FREE_LOCK(stack->lock);  return 0x0;}
    uint64_t item = *((uint64_t*)((uint64_t)stack->top-sizeof(uint64_t)));
    stack->top = (void*)((uint64_t)stack->top-sizeof(uint64_t));
    FREE_LOCK(stack->lock);
    *fetched = true;
    return item;
}

/*!
    !D Pushes item onto the stack top
    !I stack: Data stack to act on
    !I data: Data to push
    !R None
    !C LOCK: This function aquires the stack's lock
*/
void pushStack(dstack *stack, uint64_t data){
    GET_LOCK(stack->lock);
    if(((uint64_t)stack->top-(uint64_t)stack->base+sizeof(uint64_t)) > stack->maxSize){
        growStack(stack,sizeof(uint64_t)*10);
        stack->maxSize += (sizeof(uint64_t)*10);
    }
    *((uint64_t*)stack->top) = data;
    stack->top = (void*)((uint64_t)stack->top)+sizeof(uint64_t);
    FREE_LOCK(stack->lock);
}

/*!
    !D Creates a data stack
    !I initialSize: size, in bytes, to make for the stack
    !R Returns pointer to dstack on success, or NULL on failure
    !C NOTE: pushStack() Always pushes 8 bytes at a time \
    It may be a good idea to make 'initialSize' a multiple of 8
*/
dstack *createDstack(size_t initialSize){
    dstack *stack = (dstack*)kmalloc(sizeof(dstack));
    if(!stack){return NULL;}
    stack->base = (void*)kmalloc(initialSize);
    if(!stack->base){kfree(stack); return NULL;}
    stack->top = stack->base;
    stack->maxSize = initialSize;
    FREE_LOCK(stack->lock);
    return stack;
}

/*!
    !D Gets current number of items on the stack
    !I stack: Data stack to act on
    !R Returns size, in bytes, of data currently on the stack
    !C LOCK: This function aquires the lock
*/
size_t stackCount(dstack *stack){
    GET_LOCK(stack->lock);
    size_t inUse = DSTACK_BYTES_IN_USE(stack);
    FREE_LOCK(stack->lock);
    return (inUse == 0) ? 0 : inUse/sizeof(uint64_t);
}

/*!
    !D Destroys a data stack
    !I stack: Data stack to act on
    !R None
    !C LOCK: This function aquires the stack's lock
*/
void destroyDstack(dstack *stack){
    GET_LOCK(stack->lock);
    kfree(stack->base);
    kfree(stack);
}