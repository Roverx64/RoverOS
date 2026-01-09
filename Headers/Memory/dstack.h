#pragma once

#include <stdint.h>
#include <mutex.h>

#define DSTACK_BYTES_IN_USE(stack) ((uint64_t)stack->top-(uint64_t)stack->base)

typedef struct{
    void *base; //!< Base address of stack
    void *top; //!< Current stack ptr
    size_t maxSize; //!< Max number of bytes allowed
    mutex_t lock;
}dstack;

extern void shrinkStack(dstack *stack, size_t sz);
extern void growStack(dstack *stack, size_t sz);
extern uint64_t popStack(dstack *stack, bool *fetched);
extern void pushStack(dstack *stack, uint64_t data);
extern dstack *createDstack(size_t initialSize);
extern void destroyDstack(dstack *stack);
extern size_t stackCount(dstack *stack);