#include <stdint.h>
#include <port.h>
#include <kmalloc.h>
#include "pci.h"
#include "debug.h"
#include "paging.h"

struct pciInfo pci;
uint16_t alloc = 10;

uint32_t getPCIData(uint32_t bus, uint32_t dev, uint32_t fn, uint32_t reg){
    outd(PCI_CONFIG_ADDRESS,((uint32_t)1<<31)|(bus<<16)|(dev<<11)|(fn<<8)|(reg&0xFC));
    return ind(PCI_CONFIG_DATA);
}

void setPCIData(uint32_t bus, uint32_t dev, uint32_t fn, uint32_t reg, uint32_t value){
    outd(PCI_CONFIG_ADDRESS,((uint32_t)1<<31)|(bus<<16)|(dev<<11)|(fn<<8)|(reg&0xFC));
    outd(PCI_CONFIG_DATA,value);
}

void scanBus(uint8_t bus){
    for(int i = 0; i < 255; ++i){
        uint32_t vendor = getPCIData(bus,i,0,PCI_HEADER_VENDOR);
        if(vendor >= 0xFF){continue;}
        uint32_t deviceCode = 0x0;
        deviceCode |= getPCIData(bus,i,0,PCI_HEADER_CLASS);
        deviceCode |= (getPCIData(bus,i,0,PCI_HEADER_SUBCLASS)<<8);
        deviceCode |= (getPCIData(bus,i,0,PCI_HEADER_DEVICEID)<<16);
        pci.device[pci.deviceCount].deviceCode = deviceCode;
        pci.device[pci.deviceCount].bus = bus;
        pci.device[pci.deviceCount].behindBridge = false;
        pci.device[pci.deviceCount].bridge = 0;
        kdebug(DINFO,"Found device on bus 0x%lx with code 0x%lx from vendor 0x%lx\n",bus,deviceCode,vendor);
        ++pci.deviceCount;
        //Ensure we have room for more
        if(pci.deviceCount >= alloc){
            alloc += 10;
            pci.device = (struct pciDeviceInfo*)krealloc(pci.device,sizeof(struct pciDeviceInfo)*alloc);
        }
    }
}

void initPCI(){
    kdebug(DINFO,"Scanning PCI\n");
    pci.device = (struct pciDeviceInfo*)kmalloc(sizeof(struct pciDeviceInfo)*alloc);
    scanBus(0); //Qemu has everything on this bus, but this should be changed to check other buses
    pci.device = (struct pciDeviceInfo*)krealloc(pci.device,sizeof(struct pciDeviceInfo)*pci.deviceCount);
    kdebug(DINFO,"Enumerated 0x%lx PCI devices\n",(uint64_t)pci.deviceCount);
}
