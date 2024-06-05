#include <stdint.h>
#include <kernel.h>
#include <debug.h>

void kpanicInt(const char *func, const uint64_t line, const char *str, uint64_t ex){
    asm volatile("cli");
    kdebug(DNONE,"!!!#=========KERNEL=PANIC=========#!!!\n");
    kdebug(DNONE,"Cause: %s\n",str);
    kdebug(DNONE,"Caller: %s\n",func);
    kdebug(DNONE,"Line: 0x%lx\n",line);
    kdebug(DNONE,"EX: 0x%lx\n",ex);
    kdebug(DNONE,"!!!#==============================#!!!\n");
    for(;;){asm volatile("hlt");}
}