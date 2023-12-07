#include <stdint.h>
#include "acpi.h"
#include "lapic.h"
#include "hpet.h"
#include "debug.h"
#include "interrupt.h"
#include "kernel.h"

//TODO: Finish this and make this auto select the best timer to use

void setOneshot(uint64 ms){
    //Deadline
    if(sysinf.time.hasDeadline){
        setDeadline(ms*sysinf.time.deadlinetpms);
        return;
    }
    //Lapic oneshot
    if(sysinf.time.hasLapic){
        return;
    }
    //HPET oneshot
    if(sysinf.time.hasHpet){
        return;
    }
}

void startOneshot(uint64 ms){
    if(sysinf.time.hasDeadline){
        startDeadline();
        return;
    }
}