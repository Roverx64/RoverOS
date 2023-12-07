#include <stdint.h>
#include "ahci.h"
#include "pci.h"
#include "debug.h"
#include "mmio.h"
#include "paging.h"

void *ahcibar = NULL;
uint8 bus = 0;
uint8 dev = 0;

uint32 readAHCIPCI(uint32 reg){
    writePCIAddress(bus,dev,0,reg);
    return readPCI32(CONFIG_DATA);
}

void writeAHCIPCI(uint32 reg, uint32 data, bool preserve){
    uint32 write = 0x0;
    if(preserve){write = readAHCIPCI(reg);}
    write |= data;
    writePCIAddress(bus,dev,0,reg);
    writePCI32(reg,write);
}

uint64 readAHCI(uint64 offset, uint64 sz){
    return readMMIO(ahcibar,offset,sz);
}

void writeAHCI(uint64 offset, uint64 data, uint64 sz){
    writeMMIO(ahcibar,offset,data,sz);
}

void writeAHCIPort(uint64 port, uint64 offset, uint64 data, uint32 sz){
    writeAHCI(AHCI_PORT_CONTROL(port)+offset,data,sz);
}

uint64 readAHCIPort(uint64 port, uint64 offset, uint32 sz){
    return readAHCI(AHCI_PORT_CONTROL(port)+offset,sz);
}

void printSignature(uint32 port){
    uint32 sig = readAHCI(AHCI_PORT_CONTROL(port)+AHCI_PORT_SIG,MMIO_DWORD);
    kdebug(DNONE,"Sig: 0x%x\n",sig);
}

uint8 deviceStatus(uint32 port){
    uint64 portctrl = AHCI_PORT_CONTROL(port);
    uint64 status = readAHCI(portctrl+AHCI_PORT_SSTS,MMIO_DWORD);
    kdebug(DNONE,"Status=0x%lx\n",status);
    kdebug(DNONE,"Portctl=0x%lx->0x%lx\n",portctrl,portctrl+AHCI_PORT_SSTS);
    uint64 det = AHCI_PORT_SSTS_DET(status);
    switch(det){
        case AHCI_PORT_SSTS_DET_NONE:
        kdebug(DNONE,"No device\n");
        break;
        case AHCI_PORT_SSTS_DET_NPHYS:
        kdebug(DNONE,"No communication\n");
        break;
        case AHCI_PORT_SSTS_DET_PRES:
        kdebug(DNONE,"Present\n");
        break;
        case AHCI_PORT_SSTS_DET_OFFLINE:
        kdebug(DNONE,"Offline\n");
        break;
    }
    printSignature(port);
    return det;
}

uint8 deviceIPM(uint32 port){
    uint32 ipm = readAHCI(AHCI_PORT_CONTROL(port)+AHCI_PORT_SSTS,MMIO_DWORD);
    ipm = AHCI_PORT_SSTS_IPM(ipm);
    kdebug(DNONE,"Port%x state: ",port);
    switch(ipm){
        case AHCI_PORT_SSTS_IPM_NONE:
        kdebug(DNONE,"No device\n");
        break;
        case AHCI_PORT_SSTS_IPM_PARTIAL:
        kdebug(DNONE,"Partial\n");
        break;
        case AHCI_PORT_SSTS_IPM_ACTIVE:
        kdebug(DNONE,"Active\n");
        break;
        case AHCI_PORT_SSTS_IPM_SLUMBER:
        kdebug(DNONE,"Slumber\n");
        break;
        case AHCI_PORT_SSTS_IPM_DEVSLP:
        kdebug(DNONE,"Devslp\n");
        break;
    }
    return ipm;
}

void idlePort(uint32 port){

}

void initPort(uint32 port){

}

void sendCommand(uint32 port, uint8 command){
    uint32 write = (uint32)(command&0x0F)<<28;
    writeAHCI(AHCI_PORT_CONTROL(port)+AHCI_PORT_CMD,write,MMIO_DWORD);
}

void initAHCI(uint32 b, uint32 d){
    bus = b;
    dev = d;
    //Set bar
    uint64 ubar = (uint64)readAHCIPCI(AHCI_PCI_ABAR);
    kmapPage(KALIGN(ubar),KALIGN(ubar),true,false,false,false,PG_PCD,true);
    kdebug(DINFO,"BAR: 0x%lx\n",ubar);
    ahcibar = (void*)ubar;
    //Read version
    uint32 ver = readAHCI(AHCI_VERSION,MMIO_DWORD);
    kdebug(DINFO,"AHCI version %x.%x\n",AHCI_VERSION_MAJOR(ver),AHCI_VERSION_MINOR(ver));
    uint32 cap = readAHCI(AHCI_HOST_CAPABILITIES,MMIO_DWORD);
    uint32 xcap = readAHCI(AHCI_xHOST_CAPABILITIES,MMIO_DWORD);
    //Handoff control to us if BOH is set
    if(xcap&AHCI_xHOST_CAPABILITIES_BOH){
        uint32 boh = readAHCI(AHCI_BIOS_HANDOFF_CS,MMIO_DWORD);
        if(!(boh&AHCI_BIOS_HANDOFF_CS_OOS)){
            writeAHCI(AHCI_BIOS_HANDOFF_CS,AHCI_BIOS_HANDOFF_CS_OOS,MMIO_DWORD);
            kdebug(DINFO,"Obtained HBA control from the BIOS\n");
        }
    }
    //Enable AHCI
    uint32 ghc = readAHCI(AHCI_GLOBAL_HOST_CONTROL,MMIO_DWORD);
    if((ghc&AHCI_GLOBAL_HOST_CONTROL_AE) == 0x0){
        kdebug(DINFO,"Enabling AHCI\n");
        writeAHCI(AHCI_GLOBAL_HOST_CONTROL,AHCI_GLOBAL_HOST_CONTROL_AE,MMIO_DWORD);
    }
    kdebug(DINFO,"AHCI is enabled\n");
    //Check for 64 bit support
    if(readAHCI(AHCI_HOST_CAPABILITIES,MMIO_DWORD)&AHCI_HOST_CAPABILITIES_S64A){kdebug(DINFO,"64 bit supported\n");}
    //Reset ports and HBA
    kdebug(DINFO,"Resetting ports and HBA\n");
    uint64 imp = readAHCI(AHCI_HOST_CAPABILITIES,MMIO_DWORD)&AHCI_HOST_CAPABILITIES_NPS;
    uint64 bmt = readAHCI(AHCI_PORTS_IMPLEMENTED,MMIO_DWORD);
    kdebug(DINFO,"0x%lx ports | 0x%lx bitmap\n",imp,bmt);
    //Perform HBA reset
    writeAHCI(AHCI_GLOBAL_HOST_CONTROL,0x1,MMIO_DWORD);
    //Sleep for a second and then check status
    //sleep(1);
    for(;(readAHCI(AHCI_GLOBAL_HOST_CONTROL,MMIO_DWORD)&0x1) != 0x0;){
        //sleep(1);
    }
    kdebug(DINFO,"HBA reset completed\n");
    if(readAHCI(AHCI_HOST_CAPABILITIES,MMIO_DWORD)&AHCI_HOST_CAPABILITIES_SSS){kdebug(DINFO,"Skipping port reset\n"); goto skip;}
    //Reset ports
    for(int p = 0; p < 32; ++p){
        if((bmt&(1<<p)) == 0x0){continue;}
        //Reset port
        writeAHCIPort(p,AHCI_PORT_SCTL,AHCI_PORT_SCTL_DET_COMRESET,MMIO_DWORD);
    }
    //Sleep to ensure COMREST is sent
    //sleep(3);
    kdebug(DINFO,"Reset all ports\n");
    skip:
    //Check port status
    //for(int p = 0; p < 32; ++p){
    //    if((bmt&(1<<p)) == 0x0){continue;}
    //    deviceStatus(p);
    //}
}
