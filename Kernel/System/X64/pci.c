#include <stdint.h>
#include <port.h>
#include "pci.h"
#include "debug.h"
#include "paging.h"

uint32 readPCI32(uint16 addr){
    return ind(addr);
}

void writePCI32(uint16 addr, uint32 data){
    outd(addr,data);
}

void writePCIAddress(uint8 bus, uint8 dev, uint8 fn, uint8 reg){
    uint32 bus32 = (uint32)bus;
    uint32 dev32 = (uint32)dev;
    uint32 fn32 = (uint32)fn;
    uint32 reg32 = (uint32)reg;
    writePCI32(CONFIG_ADDRESS,((uint32)1<<31)|(bus32<<16)|(dev32<<11)|(fn32<<8)|(reg32&0xFC));
}

uint16 devn = 0;

void scanBus(uint8 bus){
    uint8 devices[257];
    for(uint16 i = 0; i <= 255; ++i){
        writePCIAddress(bus,i,0,0);
        uint32 vendor = readPCI32(CONFIG_DATA)&0xFFFF;
        if((vendor&0xFFFF) == 0xFFFF){continue;}
        devices[devn] = i;
        ++devn;
        writePCIAddress(bus,i,0,PCI_HEADER_CLASS);
        uint32 class = readPCI32(CONFIG_DATA);
        uint32 subclass = (class&0xFF0000)>>16;
        class = (class&0xFF000000)>>24;
        writePCIAddress(bus,i,0,PCI_HEADER_DEVICEID);
        uint32 devid = (readPCI32(CONFIG_DATA)&0xFFFF0000)>>16;
        writePCIAddress(bus,i,0,PCI_HEADER_SUBCLASS);
        uint32 subsys = (readPCI32(CONFIG_DATA)&0xFFFF0000)>>16;
        kdebug(DNONE,"[B:0x%x]-->[P:0x%x]->[V:%x|C:%x|S:%x|D:%x|Y:%x]",bus,(uint32)i,vendor,class,subclass,devid,subsys);
        //TODO: Detect XHCI,AHCI,etc based on the other PCI fields
        if(vendor == 0x1b36 && devid == 0xd){kdebug(DNONE," XHCI\n"); initXHCI(bus,i); continue;}
        if(vendor == 0x1234 && devid == 0x1111){kdebug(DNONE," BGA");}
        if(vendor == PCI_VENDOR_INTEL && devid == 0x1237){kdebug(DNONE," PMC Natoma");}
        if(vendor == PCI_VENDOR_INTEL && devid == 0x7000){kdebug(DNONE," ISA Natoma/Triton");}
        if(vendor == PCI_VENDOR_INTEL && devid == 0x100e){kdebug(DNONE," Gbit Ethernet");}
        if(vendor == PCI_VENDOR_INTEL && devid == 0x2922){kdebug(DNONE," AHCI\n"); initAHCI(bus,i); continue;}
        kdebug(DNONE,"\n");
    }
}

void initPCI(){
    kdebug(DINFO,"Scanning PCI\n");
    scanBus(0);
    kdebug(DINFO,"Enumerated 0x%x PCI devices\n",(uint32)devn);
}