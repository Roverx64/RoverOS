#include <stdint.h>
#include <kernel.h>
#include <kprint.h>
#include "syscall.h"

syscallHandler handles[2] = {0x0,0x0};

/*!
    !D Adds a function to the list of syscall handlers
    !I handle: function to call on syscall
    !I n: syscall number to use
    !R None
    !C TODO: Fix this up to handle conflicts
*/
void registerSyscall(syscallHandler handle, uint64_t n){
    kinfo("Registering handle 0x%lx to 0x%lx\n",handle,n);
    handles[n] = handle;
}

/*!
    !D Entry point for syscalls
    !I n: syscall number
    !R None
*/
void syscallEntry(uint64_t n){
    KASSERT((handles[n] == 0x0),"Unhandled syscall");
    handles[n](n);
    return;
}