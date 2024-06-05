#pragma once
#include <stdint.h>

#define MAILBOX_MAIL_TYPE_INVLD 0x0
#define MAILBOX_MAIL_TYPE_ALLOC 0x1
#define MAILBOX_MAIL_TYPE_FREE 0x2
#define MAILBOX_MAIL_TYPE_MAP 0x3
#define MAILBOX_MAIL_TYPE_UNMAP 0x4

typedef struct{
    uint16_t id; //Primarily for sender's use
    uint8_t type; //Type of mail
    uint8_t flags; //Flags
    uint64_t *data; //Pointer to mail's buffer
}mail;

typedef struct{
    mail *mail; //Message buffer
    uint64_t mailTop; //Current mail pointer
    mutex_t inUse; //Locked when either kernel/driver is acessing the mailbox
}mailbox;

struct messageAlloc{
    union{
    size_t sz; //Set by sender
    uint64_t ptr; //Set by kernel
    };
}__attribute__((packed));

struct messageFree{
    uint64_t ptr; //Set to 0x0 when kernel is done
}__attribute__((packed));

struct messageMap{
    uint64_t ptr;
    uint16_t flags;
}__attribute__((packed));

struct messageUnmap{
    uint64_t ptr;
}__attribute__((packed));