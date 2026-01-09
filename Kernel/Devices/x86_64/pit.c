#include <stdint.h>
#include <kprint.h>
#include <port.h>
#include <cpu.h>
#include <pit.h>

bool pitirq = false;

void pitInterrupt(registers r){
    pitirq = true;
    return;
}

void pitOneShot(uint16_t ticks){
    pitirq = false;
    ticks = 1193180/ticks;
    outb(PIT_PORT_CHANNEL0,(uint8_t)(ticks&0xFF)); //Low byte
    outb(PIT_PORT_CHANNEL0,(uint8_t)((ticks&0xFF00)>>8)); //High byte
}

void pitCalibrate(uint16_t ticks){
    pitOneShot(ticks);
    while(!pitirq){asm volatile("hlt");}
}

//Since the pit would only be used for task switching or calibration,
//only using this mode is fine
void initPIT(){
    kinfo("Initilizing PIT\n");
    addInterruptHandler(0x20,pitInterrupt); //IRQ0
    uint8_t cmd = PIT_MODE_OPERATING(PIT_OPERATING_MODE_INT_ON_COUNT);
    cmd |= PIT_MODE_ACCESS(PIT_ACCESS_HILO)|PIT_MODE_CHANNEL(PIT_CHANNEL0);
    outb(PIT_PORT_MODE,cmd);
    kinfo("Initilized PIT with command [0x%hx]\n",cmd);
}

//00110000