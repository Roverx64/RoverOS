#include <stdint.h>
#include "pci.h"
#include "xhci.h"
#include "paging.h"
#include "debug.h"
#include "pmm.h"
#include "vmm.h"
#include "mmio.h"
#include "kheap.h"
#include "sleep.h"

void *xhcibar; //BAR
void *xhcicap; //Capabilities
void *xhciops; //Operational space
uint8 xhcibus = 255;
uint8 xhcidev = 255;
uint64 cmdtrbEnqeue = 0;

uint32 readXHCIPCI(uint32 reg){
    writePCIAddress(xhcibus,xhcidev,0,reg);
    return readPCI32(CONFIG_DATA);
}

void writeXHCI(void *bar, uint64 v, uint64 offset, uint64 sz){
    writeMMIO(bar,offset,v,sz);
}

uint32 readXHCI(void *bar, uint64 offset,uint64 sz){
    return readMMIO(bar,offset,sz);
}

void writeXHCICommand(void *trb, void *data, size sz){
    memcpy((void*)((uint64)trb+(cmdtrbEnqeue*16)),data,sz);
    cmdtrbEnqeue++;
}



void initXHCI(uint8 bus, uint8 slot){
    return;
    kdebug(DNONE,"#===============XHCI===============#\n");
    xhcibus = bus;
    xhcidev = slot;
    //Set BAR
    uint32_t bar0 = readXHCIPCI(XHCI_PCI_BAR0);
    uint32_t bar1 = readXHCIPCI(XHCI_PCI_BAR1);
    uint64_t ubar = (((uint64)bar1<<32))|bar0;
    xhcibar = (void*)kallocatePages(KALIGN(ubar),true,false,false,false,1,PG_PCD);
    kmapPage((uint64_t)xhcibar,(uint64_t)xhcibar,true,false,false,false,0x0,true);
    xhcibar = (void*)((uint64_t)xhcibar+KALIGN(ubar));
    kdebug(DNONE,"BAR: 0x%lx->0x%lx\n",ubar,(uint64_t)xhcibar);
    //Debug
    uint32_t usbv = XHCI_PCI_DBRN_SBRN(readXHCIPCI(XHCI_PCI_DBRN));
    kdebug(DNONE,"USB %x.%x compliant\n",(usbv>>4),usbv&0x0F);
    uint32_t cap = readXHCIPCI(XHCI_PCI_CAPABILITIES)&XHCI_PCI_CAPABILITIES_MASK;
    kdebug(DNONE,"Capabilities at 0x%x\n",cap);
    //Setup
    xhcicap = (void*)((uint64_t)xhcibar+cap);
    uint8_t len = readXHCI(xhcicap,XHCI_CAPABILITIES_LENGTH,MMIO_BYTE);
    xhciops = (void*)((uint64_t)xhcicap+len);
    //Reset to known values
    writeXHCI(xhciops,XHCI_OPERATION_REGISTER_USBCMD_HCRST,XHCI_OPERATION_REGISTER_USBCMD,MMIO_DWORD);
    kdebug(DNONE,"Sent reset signal to host controller\n");
    for(uint32 b = 0;(b&XHCI_OPERATION_REGISTER_USBCMD_HCRST) != 0x0;){b = readXHCI(xhciops,XHCI_OPERATION_REGISTER_USBCMD,MMIO_DWORD);}
    for(uint32 c = 0;(c&XHCI_OPERATION_REGISTER_USBSTS_CNR) != 0x0;){c = readXHCI(xhciops,XHCI_OPERATION_REGISTER_USBSTS,MMIO_DWORD);}
    kdebug(DNONE,"Controller reset completed\n");
    //Set device slots
    writeXHCI(xhciops,0x1,XHCI_OPERATION_REGISTER_CONFIG,MMIO_BYTE);
    //Initilize DCBAAP
    void *dcbaap = (void*)halloc(0x448);
    dcbaap = (void*)(((uint64)dcbaap+0x48)&0xFFFFFFFFFFFFFFC0);
    memset(dcbaap,0x0,0x448);
    writeXHCI(xhciops,(uint64)dcbaap,XHCI_OPERATION_REGISTER_DCBAAP,MMIO_QWORD);
    kdebug(DNONE,"Set DCBAAP to 0x%lx\n",(uint64)dcbaap);
    //Set Command TRB
    void *cmdtrb = (void*)halloc(0xA4);
    cmdtrb = (void*)(((uint64)cmdtrb+0x48)&0xFFFFFFFFFFFFFFC0);
    kdebug(DNONE,"Command TRB at 0x%lx\n",(uint64)cmdtrb);
    writeXHCI(xhciops,(uint64_t)cmdtrb,XHCI_OPERATION_REGISTER_CRCR,MMIO_QWORD);
    //Set event TRB
    void *evnttrb = (void*)halloc(0xA4);
    evnttrb = (void*)(((uint64)evnttrb+0x48)&0xFFFFFFFFFFFFFFC0);
    kdebug(DNONE,"Event TRB at 0x%lx\n",(uint64)evnttrb);
    //Write No-op
    void *dat = halloc(sizeof(uint64)*2);
    ((uint32*)dat)[0] = XHCI_COMMAND_NOOP(0,XHCI_COMMAND_TRB_ID_NOOP);
    memset(dat,0x0,sizeof(uint64)*2);
    writeXHCICommand(cmdtrb,dat,sizeof(uint64)*2);
    hfree(dat);
    //Allow command execution
    //writeXHCI(xhciops,XHCI_OPERATION_REGISTER_USBCMD_RS,XHCI_OPERATION_REGISTER_USBCMD,MMIO_DWORD);
    //sleep(100);
    //Test for event
    kdebug(DNONE,"#==================================#\n");
}