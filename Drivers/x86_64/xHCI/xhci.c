#include <stdint.h>
#include <klib.h>
#include <kcache.h>
#include <kmalloc.h>
#include <pci.h>
#include <cpu.h>

kcache *xhciCache = NULL;
struct xhciDeviceInfo xhci;

void xhciCacheConstructor(void *ptr){
    return;
}

void xhciCacheDestructor(void *ptr){
    return;
}

void checkForMSIX(){
    //Check for capabilities list
    uint32_t status = PCI_HEADER_STATUS((getPCIData(xhci.bus,xhci.dev,0,PCI_HEADER_STATUS_COMMAND)));
    if(status&PCI_HEADER_STATUS_CAPABILITIES_LIST == 0){
        xhci.msi.hasMSIX = false;
        return;
    }
    //Parse list
    uint32_t capability = getPCIData(xhci.bus,xhci.dev,0,PCI_HEADER_CAPABILITIES_PTR_RESERVED);
    capability = PCI_CAPABILITIES_POINTER(capability);
    while(capability != 0x0){
        uint32_t check = getPCIData(xhci.bus,xhci.dev,0,capability);
        if(check&0xFF == PCI_CAPABILITY_MSIX){
            xhci.msi.hasMSIX = true;
            xhci.msi.msixAddress = capability;
            return;
        }
        capability = (check>>8)&0xFF;
    }
}

void initEventRing(){
    //Allocate segment table
    rdmmio(xhci.);
    struct xhciEventRingSegment *segments = (struct xhciEventRingSegment*)kcacheAlloc(xhciCache,0);
    memset(xhci.eventRing.segments,0x0,PAGE_SZ);
    //Write to interrupter 0
    void *bar = (void*)XHCI_INTERRUPTER_BAR(xhci.runtimeBAR,0);
    uint64_t wr = (uint64_t)segments<<5;
    wlmmio(bar,XHCI_INTERRUPTER_ERSTBA,wr);
    wlmmio(bar,XHCI_INTERRUPTER_ERSTSZ,PAGE_SZ/sizeof(uint64_t));
}

void initScratchpads(){

}

void initMSIX(){
    //Get BAR for MSI-X
    uint32_t BARn = getPCIData(xhci.bus,xhci.dev,0,xhci.msixAddress+PCI_MSIX_ROW_1);
    uint32_t offset = PCI_MSIX_TABLE_OFFSET(BARn);
    BARn = PCI_MSIX_TABLE_BIR(BARn);
    xhci.msi.msixBAR = (void*)(getPCIData(xhci.bus,xhci.dev,0,BARn)+offset);
    //Get table size
    uint32_t mctrl = getPCIData(xhci.bus,xhci.dev,0,xhci.msi.msixAddress+PCI_MSIX_ROW_0);
    mctrl = PCI_MSIX_MESSAGE_CONTROL(mctrl);
    xhci.msi.msixTableSize = PCI_MSIX_MESSAGE_CONTROL_TABLE_SZ(mctrl);
    //Get BAR for PBA
    BARn = getPCIData(xhci.bus,xhci.dev,0,xhci.msi.msixAddress+PCI_MSIX_ROW_2);
    offset = PCI_MSIX_PBA_OFFSET(BARn);
    BARn = PCI_MSIX_PBA_BIR(BARn);
    xhci.msi.pbaBAR = (void*)(getPCIData(xhci.bus,xhci.dev,0,BARn)+offset);
    //Enable MSI-X 
    mctrl = PCI_MSIX_MESSAGE_CONTROL_ENABLE;
    setPCIData(xhci.bus,xhci.dev,0,xhci.msi.msixAddress+PCI_MSIX_ROW_0,mctrl);
    return;
}

void initMSI(){
    //
}

void initXHCI(void *bar, uint8_t bus, uint8_t dev){
    xhci.bus = bus;
    xhci.dev = dev;
    xhciCache = kcacheCreate("xhciCache",PAGE_SZ,0,0,cacheConstructor,cacheDestructor);
    //Reset xHCI to a known state
    xhci.capabilitiesBAR = bar;
    rlmmio(xhci.capabilitiesBAR,XHCI_CAPABILITIES_LENGTH,xhci.capLen);
    rlmmio(xhci.capabilitiesBAR,XHCI_CAPABILITIES_RSOFF,xhci.runtimeOffset);
    xhci.hostOpBAR = (void*)XHCI_OPERATION_REGISTER_SPACE(xhci.capabilitiesBAR,xhci.capLen);
    wlmmio(xhci.hostOpBAR,XHCI_OPERATION_REGISTER_USBCMD,XHCI_OPERATION_REGISTER_USBCMD_HCRST);
    uint64_t test = 1;
    while(test){
        rlmmio(xhci.hostOpBAR,XHCI_OPERATION_REGISTER_USBCMD);
        test &= XHCI_OPERATION_REGISTER_USBCMD_HCRST;
    }
    //Get required information
    rlmmio(xhci.capabilitiesBAR,XHCI_CAPABILITIES_DBOFF,xhci.doorbellOffset);
    rlmmio(xhci.capabilitiesBAR,XHCI_CAPABILITIES_RSOFF,xhci.runtimeOffset);
    xhci.runtimeBAR = (void*)((uint64_t)xhci.capabilitiesBAR+xhci.runtimeOffset);
    //Setup device slots
    rlmmio(xhci.hostOpBAR,XHCI_CAPABILITIES_HCSPARAMS1,xhci.maxSlots);
    xhci.maxSlots = XHCI_CAPABILITIES_HCSPARAMS1_MAX_SLOTS(xhci.maxSlots);
    //Enable slots
    uint64_t cnfg = 0x0;
    rlmmio(xhci.hostOpBAR,XHCI_OPERATION_REGISTER_CONFIG,cnfg);
    cnfg = (cnfg&0x300)|xhci.maxSlots;
    wlmmio(xhci.hostOpBAR,XHCI_OPERATION_REGISTER_CONFIG,cnfg);
    //Set DCB address
    xhci.dcb = (void*)kmalloc(xhci.maxSlots*sizeof(uint64_t));
    memset(xhci.dcb,0x0,xhci.maxSlots*sizeof(uint64_t));
    wlmmio(xhci.hostOpBAR,XHCI_OPERATION_REGISTER_DCBAAP,xhci.dcb);
    //Set command queue address
    xhci.cmdQueueSz = 10;
    xhci.commandQueue = (uint64_t*)kmalloc(sizeof(uint64_t)*xhci.cmdQueueSz);
    uint64_t wr = (uint64_t)xhci.commandQueue<<6;
    wlmmio(xhci.hostOpBAR,XHCI_OPERATION_REGISTER_CRCR,wr);
    //Initilize interrupts
    //MSI-X is prefered
    checkForMSIX();
    if(xhci.msi.hasMSIX){initMSIX();}
    else{initMSI();}
    //Initilize queues
    initEventRing();
    //Start the controller
    wlmmio(xhci.hostOpBAR,XHCI_OPERATION_REGISTER_USBCMD,XHCI_OPERATION_REGISTER_USBCMD_RS);
}