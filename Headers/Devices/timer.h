#pragma once
#include <stdint.h>

//The lower the number, the more prefered
#define TIMER_TYPE_DEADLINE 0 //Lapic tsc deadline
#define TIMER_TYPE_LAPIC 1
#define TIMER_TYPE_HPET 2 //1 per availible HPET timer
#define TIMER_TYPE_PIT 0xFF //Worst option possible

//Args: event id
typedef void (*timerCallback)(uint64_t);

//Args: time
//All time is in nanoseconds
//Round to nearest allowed time if ns precision is not availible
typedef void (*timerSet)(uint64_t);

//Info for the best timer found
struct timerInfo{
    uint8_t type;
    timerSet set;
};

//Min-heap struct
struct timerEvent{
    uint64_t ns; //Time to wait in ns (deincrimented and executed if <= 0)
    uint64_t id; //Unique ID set by the caller
    timerCallback callback; //Function to call on event end
};

//Must be an even number (use %2 to find out)
#define TIMER_PARENT(e) ((e>>1)-1)
#define TIMER_RIGHT_ENTRY(e) ((e<<1)+2)
#define TIMER_LEFT_ENTRY(e) ((e<<1)+1)

extern void initTimers();
extern void addTimerEvent(uint64_t ns, uint64_t id, timerCallback call);
extern void registerTimer(uint8_t type, timerSet set);
extern void timerEnd();