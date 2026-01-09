#pragma once
#include <stdint.h>

//Primarily for driver to user communication

// 0x0 - 0xFFFF is kernel defined
// 0x10000 - 0xFFFFFFFFFF is driver defined
// 0x10000000000 - 0xFFFFFFFFFFFFFFFF is user defined

#define HOOK_IO_READY 0 //IO data is ready to be read
#define HOOK_END 1 //The device has disconnected/is no longer functional
#define HOOK_BEGIN 2 //The device is initilizing
#define HOOK_EVENT 3 //

//This is for one way communication (I.E keyboard/mouse input)
struct hook{
    uint64_t id; //Type of hook
    void *msgQueue; //Message queue (Mapped into user mem as RO)
    size_t queueSize; //Bytes allocated
    uint64_t cursor; //Current position in the queue
};

//Base struct required by all messages
struct hookMessage{
    uint8_t type; //Type of message
    size_t sz; //length of message
};

struct hookLibrary{
    uint16_t id;
    struct hook *hooks;
    size_t hookCount;
};