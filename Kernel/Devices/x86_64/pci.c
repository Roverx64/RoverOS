#include <stdint.h>
#include <port.h>
#include <kmalloc.h>
#include <kcache.h>
#include <kernel.h>
#include "pci.h"
#include "kprint.h"
#include "paging.h"

struct pciInfo pci;
uint16_t alloc = 10;
kcache *msiCache;
kcache *msixCache;

uint32_t getPCIData(uint32_t bus, uint32_t dev, uint32_t fn, uint32_t reg){
    outd(PCI_CONFIG_ADDRESS,((uint32_t)1<<31)|(bus<<16)|(dev<<11)|(fn<<8)|(reg&0xFC));
    return ind(PCI_CONFIG_DATA);
}

void setPCIData(uint32_t bus, uint32_t dev, uint32_t fn, uint32_t reg, uint32_t value){
    outd(PCI_CONFIG_ADDRESS,((uint32_t)1<<31)|(bus<<16)|(dev<<11)|(fn<<8)|(reg&0xFC));
    outd(PCI_CONFIG_DATA,value);
}

void printVendor(uint32_t vendor){
    switch(vendor){
        case PCI_VENDOR_INTEL:
        knone(PCI_VENDOR_INTEL_STR);
        break;
        case PCI_VENDOR_BOCHS:
        knone(PCI_VENDOR_BOCHS_STR);
        break;
        case PCI_VENDOR_REDHAT:
        knone(PCI_VENDOR_REDHAT_STR);
        break;
        default:
        knone("0x%x ",vendor);
        break;
    }
}

void printDeviceName(uint32_t classcode){
    switch(classcode){
        case PCI_DEVICE_AHCI:
        knone("(%s)\n",PCI_DEVICE_AHCI_STR);
        break;
        case PCI_DEVICE_XHCI:
        knone("(%s)\n",PCI_DEVICE_XHCI_STR);
        break;
        case PCI_DEVICE_BGA:
        knone("(%s)\n",PCI_DEVICE_BGA_STR);
        break;
        case PCI_DEVICE_ETHERNET:
        knone("(%s)\n",PCI_DEVICE_ETHERNET_STR);
        break;
        case PCI_DEVICE_HOST_BRIDGE:
        knone("(%s)\n",PCI_DEVICE_HOST_BRIDGE_STR);
        break;
        case PCI_DEVICE_ISA_BRIDGE:
        knone("(%s)\n",PCI_DEVICE_ISA_BRIDGE_STR);
        break;
        default:
        knone("(Unknown)\n");
        break;
    }
}

void scanBus(uint8_t bus){
    for(int i = 0; i < 255; ++i){
        uint32_t vendor = PCI_HEADER_GET_VENDOR(getPCIData(bus,i,0,PCI_HEADER_VENDOR));
        if(vendor == 0xFFFF){continue;}
        uint64_t devid = PCI_HEADER_GET_DEVICEID(getPCIData(bus,i,0,PCI_HEADER_DEVICEID));
        uint32_t class = PCI_HEADER_GET_CLASSCODE(getPCIData(bus,i,0,PCI_HEADER_CLASSCODE));
        uint64_t base = PCI_CLASSCODE_BASE(class);
        uint64_t subClass = PCI_CLASSCODE_SUBCLASS(class);
        uint64_t interface = PCI_CLASSCODE_INTERFACE(class);
        knone("[");
        printVendor(vendor);
        knone("|0x%lx|(0x%lx)0x%lx->0x%lx:0x%lx] ",devid,class,base,subClass,interface);
        printDeviceName(class);
        pci.device[pci.deviceCount].classCode = class;
        pci.device[pci.deviceCount].bus = bus;
        pci.device[pci.deviceCount].behindBridge = false;
        pci.device[pci.deviceCount].bridge = 0;
        pci.device[pci.deviceCount].dev = i;
        ++pci.deviceCount;
        //Ensure we have room for more
        if(pci.deviceCount >= alloc){
            alloc += 10;
            pci.device = (struct pciDeviceInfo*)krealloc(pci.device,sizeof(struct pciDeviceInfo)*alloc);
        }
    }
}

extern void initAHCI(uint8_t bus, uint8_t dev);

void initPCI(){
    kinfo("Scanning PCI\n");
    pci.device = (struct pciDeviceInfo*)kmalloc(sizeof(struct pciDeviceInfo)*alloc);
    scanBus(0); //Qemu has everything on this bus, but this should be changed to check other buses
    pci.device = (struct pciDeviceInfo*)krealloc(pci.device,sizeof(struct pciDeviceInfo)*pci.deviceCount);
    kinfo("Enumerated 0x%lx PCI devices\n",(uint64_t)pci.deviceCount);
    //Check for AHCI and start it
    for(uint16_t i = 0; i < pci.deviceCount; ++i){
        if(pci.device[i].classCode == PCI_DEVICE_AHCI){
            initAHCI(pci.device[i].bus,pci.device[i].dev);
            return;
        }
    }
    //Create MSI cache
    //Should be naturally DWORD aligned
    msiCache = kcacheCreate("msi_cache",0x100,4,MEMORY_TYPE_LOW,0,NULL,NULL);
    KASSERT((msiCache != NULL),"Failed to create MSI cache");
}

//MSI/-X stuff

//Returns false if there is no MSI or MSI-X
//NOTE: It is still up to the caller to enable MSI interrupts
bool initMSIX(struct pciMSIInfo *msi, uint32_t bus, uint32_t dev, uint32_t fn){
    uint32_t cap = getPCIData(bus,dev,fn,PCI_HEADER_CAPABILITIES_PTR_RESERVED);
    uint32_t next = PCI_HEADER_CAPABILITIES_POINTER(cap);
    //Loop through pointers for MSI or MSIX
    bool found = false;
    uint32_t ptr = 0x0;
    while(next != 0x0){
        cap = getPCIData(bus,dev,0,next);
        uint8_t id = PCI_CAPABILITY_ID(cap);
        if(id == PCI_CAPABILITY_ID_MSI){msi->msix = false; found = true; ptr = next;}
        //MSI-X is preferred
        if(id == PCI_CAPABILITY_ID_MSIX){msi->msix = true; found = true; ptr = next; break;}
        next = PCI_CAPABILITY_NEXT(cap);
    }
    if(!found){return false;}
    msi->bus = bus;
    msi->dev = dev;
    msi->fn = fn;
    msi->ptr = ptr;
    uint32_t mctrl = getPCIData(bus,dev,fn,ptr+PCI_MSI_ROW0);
    mctrl = PCI_MSI_MCONTROL(mctrl);
    if(msi->msix){goto msix;}
    //=======MSI=======
    uint8_t vec = PCI_MSI_MCONTROL_VECTORS_REQUESTED(mctrl);
    msi->extended = PCI_MSI_MCONTROL_EXTENDED_ADDRESSING(mctrl);
    uint8_t mt = (msi->extended == 1) ? MEMORY_TYPE_ANY : MEMORY_TYPE_LOW;
    msi->pvMasking = PCI_MSI_MCONTROL_PER_VECTOR_MASKING(mctrl);
    //Allocate memory
    uint64_t phys = (uint64_t)kcacheAlloc(msiCache,0);
    KASSERT((phys != 0x0),"Failed to allocate pmem for MSI vectors");
    uint64_t virt = (uint64_t)vmallocPhys(phys,vec*sizeof(uint32_t),VTYPE_MMIO,VFLAG_MAKE|VFLAG_WRITE);
    KASSERT((virt != 0x0),"Failed to allocate vmem for MSI vectors");
    //Set vector address and allocated
    mctrl = (mctrl&(~0x70))|PCI_MSI_MCONTROL_VECTORS_ALLOCATED(vec);
    setPCIData(bus,dev,fn,ptr,mctrl);
    setPCIData(bus,dev,fn,ptr+PCI_MSI_MADDR_LOWER,phys&0xFFFFFFFF);
    if(msi->extended){setPCIData(bus,dev,fn,ptr+PCI_MSI_MADDR_UPPER,phys>>32);}
    return true;
    //=======MSIX=======
    msix:
    msi->tblSize = PCI_MSIX_MCONTROL_TABLE_SIZE(mctrl);
    //Read MSI-X bar
    uint32_t tblBir = getPCIData(bus,dev,fn,ptr+PCI_MSIX_TABLE);
    uint32_t tblOff = PCI_MSIX_TABLE_OFF(tblBir);
    tblBir = PCI_MSIX_TABLE_BIR(tblBir);
    uint32_t tblBar = getPCIData(bus,dev,fn,PCI_HEADER_BAR(tblBir))+tblOff;
    KASSERT(((tblBar&0x7) == 0),"MSIX table is not QWORD aligned");
    msi->tblPhys = tblBar;
    size_t bytes = (sizeof(uint64_t)*2)*msi->tblSize;
    msi->msixTable = (struct pciMSIXTbl*)vmallocPhys(tblBar,bytes,VTYPE_MMIO,VFLAG_MAKE|VFLAG_UC|VFLAG_WRITE);
    KASSERT((msi->msixTable != NULL),"valloc() failed to map MSIX table");
    //Read MSI-X PBA
    uint32_t pbaBir = getPCIData(bus,dev,fn,ptr+PCI_MSIX_PBA);
    uint32_t pbaOff = PCI_MSIX_PBA_OFF(pbaBir);
    pbaBir = PCI_MSIX_PBA_BIR(pbaBir);
    uint32_t pbaBar = getPCIData(bus,dev,fn,PCI_HEADER_BAR(pbaBir))+pbaOff;
    KASSERT(((pbaBar&0x7) == 0),"MSIX PBA is not QWORD aligned");
    msi->pbaPhys = pbaBar;
    bytes = sizeof(uint64_t)*(msi->tblSize/64);
    msi->pba = (uint64_t*)vmallocPhys(pbaBar,bytes,VTYPE_MMIO,VFLAG_MAKE|VFLAG_UC|VFLAG_WRITE);
    KASSERT((msi->pba != NULL),"valloc() failed to map MSIX PBA");
    //Enable MSI-X
    return true;
}

//Enables MSI/X interrupts
void enableMSIX(struct pciMSIInfo *msi){
    KASSERT((msi != NULL),"MSI is not supported");
    uint32_t mctrl = getPCIData(msi->bus,msi->dev,msi->fn,msi->ptr);
    mctrl |= (msi->msix == true) ? PCI_MSIX_MCONTROL_ENABLE : PCI_MSI_MCONTROL_ENABLE;
    setPCIData(msi->bus,msi->dev,msi->fn,msi->ptr,mctrl);
}

void maskMSIVector(struct pciMSIInfo *msi){
    if(msi->pvMasking){

    }
}