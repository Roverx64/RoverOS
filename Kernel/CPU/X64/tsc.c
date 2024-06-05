#include <stdint.h>
#include <cpuid.h>

//struct tscDeviceInfo tsc;

void initTSC(){
    //Check for TSC
    uint32_t check;
    uint32_t trash;
    //CPUID(1,0,&check,&trash,&trash,&trash);
    //if(check&CPUID_TSC_PRESENT){tsc.present = true;}
    //if(check&CPUID_TSC_INVARIANT){tsc.invariant = true;}
    
}