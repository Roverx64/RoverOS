#pragma once
#include <stdint.h>
#include <mutex.h>
#include <task.h>

struct mail{
    uint16_t sender; //Return 'address' (mailbox id)
};

struct mailbox{
    void *buffer;
};