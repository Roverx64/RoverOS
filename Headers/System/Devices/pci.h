#pragma once
#include <stdint.h>

struct pciBridgeInfo{

};

struct pciDeviceInfo{
    uint16_t deviceCode;
    uint8_t bus;
    bool behindBridge;
    uint8_t bridge;
};

struct pciInfo{
    uint16_t deviceCount;
    struct pciDeviceInfo *device;
    uint16_t bridgeCount;
    struct pciBridgeInfo *bridge;
};

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC
#define PCI_HEADER_VENDOR 0x0
#define PCI_HEADER_CLASS 0x8
#define PCI_HEADER_SUBCLASS 0x2C
#define PCI_HEADER_DEVICEID 0x0
#define PCI_HEADER_CAPABILITIES_PTR_RESERVED 0x34
#define PCI_HEADER_CAPABILITIES_POINTER(ptrsv) (ptrsv&0xFC)
#define PCI_HEADER_STATUS_COMMAND 0x04
#define PCI_HEADER_STATUS(scmd) (scmd>>16)
#define PCI_HEADER_STATUS_CAPABILITIES_LIST 1<<4
#define PCI_HEADER_COMMAND(scmd) (scmd&0xFFFF)

#define PCI_VENDOR_INTEL 0x8086

#define PCI_CAPABILITY_MSIX 0x11

//MSI-X structure
#define PCI_MSIX_ROW_0 0x0
#define PCI_MSIX_ROW_1 0x4
#define PCI_MSIX_ROW_2 0x8
#define PCI_MSIX_CAPABILITY_ID(fld) (fld&0xFF)
#define PCI_MSIX_NEXT_POINTER(fld) ((fld>>8)&0xFF)
#define PCI_MSIX_MESSAGE_CONTROL(fld) ((fld>>16)&0xFFFF)
#define PCI_MSIX_MESSAGE_CONTROL_ENABLE 0x1<<15
#define PCI_MSIX_MESSAGE_CONTROL_MASK 0x1<<14
#define PCI_MSIX_MESSAGE_CONTROL_TABLE_SZ(mctrl) (mctrl&0x7FF)
#define PCI_MSIX_TABLE_BIR(fld) (fld&0x7)
#define PCI_MSIX_TABLE_OFFSET(fld) (fld&0xFFFFFFF8)
#define PCI_MSIX_PBA_BIR(fld) (fld&0x7)
#define PCI_MSIX_PBA_OFFSET(fld) (fld>>3)
//Table fields
#define PCI_MSIX_MSG_ADDR_LOW(fld) (fld&0xFFFFFFFF)
#define PCI_MSIX_MSG_ADDR_HIGH(fld) (fld>>32)
#define PCI_MSIX_MSG_DATA(fld) (fld&0xFFFFFFFF)
#define PCI_MSIX_VECTOR_CONTROL(fld) (fld>>32)

extern uint32_t getPCIData(uint32_t bus, uint32_t dev, uint32_t fn, uint32_t reg);
extern void setPCIData(uint32_t bus, uint32_t dev, uint32_t fn, uint32_t reg, uint32_t value);

//PCI Devices

//[class][subclass][devid]

#define PCI_DEVICE_XHCI 0x0C0330
#define PCI_DEVICE_AHCI 0x010601
#define PCI_DEVICE_BGA 
#define PCI_DEVICE_PMC_NATOMA 
#define PCI_DEVICE_ISA_NATOMA 
#define PCI_DEVICE_GBIT_ETHERNET 

#define PCI_DEVICE_XHCI_STR "xHCI"
#define PCI_DEVICE_AHCI_STR "aHCI"
#define PCI_DEVICE_BGA_STR "BGA"
#define PCI_DEVICE_PMC_NATOMA_STR "PMC Natoma"
#define PCI_DEVICE_ISA_NATOMA_STR "ISA Natoma"
#define PCI_DEVICE_GBIT_ETHERNET_STR "Gbit ethernet"