#include <stdint.h>
#include <timer.h>
#include <sleep.h>
#include <kmalloc.h>
#include <kcache.h>
#include <string.h>

struct timerInfo timer; //Best found timer
uint32_t inQueue; //How many timers are in the queue
uint32_t next; //Next insertion spot
uint32_t qsize = 0;
struct timerEvent *queue; //Min-heap tree

//TODO: optimize rebalancing, sorting, and insertion
//NOTE: This code hasn't been tested very thoroughly yet

void resortTree(){
    //Move up lowest side and rebalance
    uint32_t idx = 0;
    uint32_t p = 0;
    uint32_t l = next; //Lowest point on the tree
    while(queue[idx].ns == 0){
        //Pull up side if needed
        lp:
        if(queue[TIMER_LEFT_ENTRY(idx)].ns >= queue[TIMER_RIGHT_ENTRY(idx)].ns){
            p = TIMER_LEFT_ENTRY(idx);
        }
        else{
            p = TIMER_RIGHT_ENTRY(idx);
        }
        if(p < l){l = p;} //Keep track of lowest point
        queue[idx].ns = queue[p].ns;
        queue[idx].callback = queue[p].callback;
        idx = p;
        goto lp; //Loop until we pull up the entire side
        //Make sure top entry is valid and keep sorting
        idx = 0;
    }
    next = l; //Next entries shall go here to keep the tree balanced
}

//Called by timer interrupt to signal the end of an event
void timerEnd(){
    uint64_t dec = queue[0].ns;
    queue[0].callback((uint64_t)&queue[0]);
    queue[0].ns = 0;
    //Deincrement timers and resort
    for(uint32_t i = 1; i < next; ++i){
        //Tree is always sorted, so 0 will always be the end of the tree
        if(queue[i].ns == 0){return;} //End of the tree
        queue[i].ns -= dec; //Child nodes cannot be smaller than the root
        //These will be fixed in the resort
        if(queue[i].ns == 0){queue[i].callback(queue[i].id); queue[i].callback = 0x0;}
    }
    resortTree();
    //Setup next timer
    timer.set(queue[0].ns);
    return;
}

void registerTimer(uint8_t type, timerSet set){
    if(timer.type < type){return;}
    timer.type = type;
    timer.set = set;
}

void addTimerEvent(uint64_t ns, uint64_t id, timerCallback call){
    if(ns == 0){call(id); return;}
    queue[next].ns = ns;
    queue[next].callback = call;
    queue[next].id = id;
    uint32_t p = TIMER_PARENT(((next%2) == 0) ? next : (next-1));
    if(queue[next].ns >= queue[p].ns){goto skip;}
    //Swap until the parent is smaller
    uint32_t v = next;
    while(queue[v].ns < queue[p].ns){
        uint64_t tmp = queue[p].ns;
        timerCallback ctmp = queue[p].callback;
        queue[p].ns = queue[v].ns;
        queue[p].callback = queue[v].callback;
        queue[v].ns = tmp;
        queue[v].callback = ctmp;
        v = p;
        p = TIMER_PARENT(((next%2) == 0) ? v : (v-1));
    }
    skip:
    ++next;
    //Loop next until free spot is found
    //Next+1 may not be free upon a resort
    while(queue[next].ns != 0){++next;}
}

void initTimers(){
    queue = (struct timerEvent*)kmalloc(sizeof(struct timerEvent)*100);
    memset(queue,0x0,sizeof(struct timerEvent)*100);
    qsize = 100;
    next = 0;
}