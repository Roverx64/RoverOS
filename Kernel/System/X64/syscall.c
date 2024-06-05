#include <stdint.h>
#include <kernel.h>
#include "debug.h"
#include "syscall.h"

syscallHandler handles[2] = {0x0,0x0};

void registerSyscall(syscallHandler handle, uint64 n){
    kdebug(DINFO,"Registering handle 0x%lx to 0x%lx\n",handle,n);
    handles[n] = handle;
}

void syscallEntry(uint64 n){
    if(handles[n] == 0x0){
        kpanic("Unhandled syscall",n);
    }
    handles[n](n);
    return;
}