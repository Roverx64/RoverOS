#include <stdint.h>
#include <timer.h>
#include <task.h>

/*!
    !D Callback timer  to unsuspend a process
    !I id: process ID
    !R None
    !C NOTE: This function is not callable
*/
timerCallback sleepEnd(uint64_t id){
    unsuspendProcess(id);
}

/*!
    !D Sleeps a process
    !I ns: nanoseconds to sleep
    !R None
    !C NOTE: Suspends the current process until the time is done
*/
void sysSleepns(uint64_t ns){
    if(ns == 0){return;}
    //Tell the scheduler to suspend the current process
    //Possible race condition if ns is really small
    uint64_t id = getCurrentTaskID();
    suspendProcess(id);
    addTimerEvent(ns,id,sleepEnd);
}

/*!
    !D MS to NS wrapper for sysSleepns()
    !I ms: Miliseconds to sleep
    !R None
*/
void sysSleep(uint64_t ms){
    sysSleepns(ms*1000000);
}