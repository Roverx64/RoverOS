#pragma once
#include <stdint.h>
#include <stdbool.h>

struct pciBridgeInfo{

};

struct pciDeviceInfo{
    uint64_t classCode;
    uint8_t bus;
    uint8_t dev;
    bool behindBridge;
    uint8_t bridge;
};

struct pciInfo{
    uint16_t deviceCount;
    struct pciDeviceInfo *device;
    uint16_t bridgeCount;
    struct pciBridgeInfo *bridge;
};

//TODO: change these names/numbers to something more sane
//considering PCI only allows DWORD reads anyways
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA 0xCFC
#define PCI_HEADER_VENDOR 0x0
#define PCI_HEADER_DEVICEID 0x0
#define PCI_HEADER_REVISIONID 0x8
#define PCI_HEADER_CLASSCODE 0x8
#define PCI_HEADER_SUBSYSTEMID 0x2C
#define PCI_HEADER_CAPABILITIES_PTR_RESERVED 0x34
#define PCI_HEADER_CAPABILITIES_POINTER(ptrsv) (ptrsv&0xFC)
#define PCI_HEADER_STATUS_COMMAND 0x04
#define PCI_HEADER_STATUS(scmd) (scmd>>16)
#define PCI_HEADER_STATUS_CAPABILITIES_LIST 1<<4
#define PCI_HEADER_COMMAND(scmd) (scmd&0xFFFF)
#define PCI_HEADER_BAR(b) (0x10+(b*4))
#define PCI_HEADER_INT 0x3C

#define PCI_HEADER_COMMAND_IO_SPACE 1<<0
#define PCI_HEADER_COMMAND_MEMORY_SPACE 1<<1
#define PCI_HEADER_COMMAND_BUS_MASTER 1<<2
#define PCI_HEADER_COMMAND_SPECIAL_CYCLES 1<<3
#define PCI_HEADER_COMMAND_MWIE 1<<4
#define PCI_HEADER_COMMAND_VGA 1<<5
#define PCI_HEADER_COMMAND_PER 1<<6
#define PCI_HEADER_COMMAND_SERR 1<<8
#define PCI_HEADER_COMMAND_FBTB 1<<9
#define PCI_HEADER_COMMAND_ID 1<<10

#define PCI_HEADER_GET_VENDOR(d) (d&0xFFFF)
#define PCI_HEADER_GET_DEVICEID(d) (d>>16)
#define PCI_HEADER_GET_CLASSCODE(d) (d>>8)
#define PCI_CLASSCODE_BASE(d) (d>>16)
#define PCI_CLASSCODE_SUBCLASS(d) ((d>>8)&0xFF)
#define PCI_CLASSCODE_INTERFACE(d) (d&0xFF)
#define PCI_HEADER_GET_SUBSYSTEMID(d) (d>>16)

#define PCI_VENDOR_INTEL 0x8086

#define PCI_CAPABILITY_ID(v) (v&0xFF)
#define PCI_CAPABILITY_NEXT(v) ((v>>8)&0xFF)

#define PCI_CAPABILITY_ID_MSI 0x5
#define PCI_CAPABILITY_ID_MSIX 0x11

//MSI/MSIX
#define PCI_MSI_ROW0 0x0
#define PCI_MSI_ROW1 0x4
#define PCI_MSI_ROW2 0x8
#define PCI_MSI_ROw3 0xC
#define PCI_MSI_ROW4 0x10

//MSI mcontrol
#define PCI_MSI_MCONTROL(fld) ((fld>>16)&0xFFFF)
#define PCI_MSI_MCONTROL_ENABLE 0x1
#define PCI_MSI_MCONTROL_VECTORS_REQUESTED(msg) ((uint32_t)1<<((msg>>1)&0x7))
#define PCI_MSI_MCONTROL_VECTORS_ALLOCATED(vec) (((uint32_t)vec&0x7)<<1)
#define PCI_MSI_MCONTROL_EXTENDED_ADDRESSING(msg) ((msg>>7)&0x1) //64-Bit capable
#define PCI_MSI_MCONTROL_PER_VECTOR_MASKING(msg) ((msg>>8)&0x1)
//MSI Base address
#define PCI_MSI_MADDR_LOWER PCI_MSI_ROW1
#define PCI_MSI_MADDR_UPPER PCI_MSI_ROW2
//MSI data
#define PCI_MSI_MDATA(x) (PCI_MSI_ROW2+(x*4)) //X is 1 if 64-Bit is enabled
//MSI Mask
#define PCI_MSI_MASK(x) (PCI_MSI_ROW3+(x*4))
//MSI Pending
#define PCI_MSI_PENDING(x) (PCI_MSI_ROW4+(x*4))

//MSIX mcontrol
#define PCI_MSIX_MCONTROL(fld) ((fld>>16)&0xFFFF)
#define PCI_MSIX_MCONTROL_TABLE_SIZE(msg) (msg&0x7FF)
#define PCI_MSIX_MCONTROL_FNMASK(msg) ((msg>>14)&0x1)
#define PCI_MSIX_MCONTROL_ENABLE 0x1<<15
//MSIX Table field
#define PCI_MSIX_TABLE PCI_MSI_ROW1
#define PCI_MSIX_TABLE_BIR(tbl) (0x10+((tbl&0x3)*4))
#define PCI_MSIX_TABLE_OFF(tbl) (tbl&(~0x3))
//MSIX PBA
#define PCI_MSIX_PBA PCI_MSI_ROW2
#define PCI_MSIX_PBA_BIR(pba) (0x10+((pba&0x3)*4))
#define PCI_MSIX_PBA_OFF(pba) (pba&(~0x3))
//MSIX MMIO Table structure
struct pciMSIXTbl{
    uint32_t msgAddrLower;
    uint32_t msgAddrUpper;
    uint32_t msgData;
    uint32_t versionCtrl;
}__attribute__((packed));

//MSIX PBA structure can be defined as a pointer to an array of qwords

extern uint32_t getPCIData(uint32_t bus, uint32_t dev, uint32_t fn, uint32_t reg);
extern void setPCIData(uint32_t bus, uint32_t dev, uint32_t fn, uint32_t reg, uint32_t value);

//Per-MSI/X function struct
struct pciMSIInfo{
    uint32_t ptr; //PCI cfg space ptr
    bool msix; //True if using MSI-X
    bool extended; //True if 64-bit addressing is allowed
    bool pvMasking; //If per-vector masking is allowed (MSI only)
    uint16_t tblSize; //MSI/MSIX vector/tbl entry count
    uint64_t tblPhys; //Phys addr of MSI/X table
    union{
        uint64_t tblVirt; //Virt addr of MSI table
        struct pciMSIXTbl *msixTable; //MMIO of MSIX table
    };
    uint64_t pbaPhys; //MSI-X only
    uint64_t *pba; //MSI-X only (virt addr)
    //PCI
    uint8_t bus;
    uint8_t dev;
    uint8_t fn;
};

//MSI helper functions
extern bool initMSIX(struct pciMSIInfo *msi, uint32_t bus, uint32_t dev, uint32_t fn);
extern void enableMSIX(struct pciMSIInfo *msi);
extern void maskMSIVector(struct pciMSIInfo *msi);

//Device codes and strings
#define PCI_DEVICE_XHCI 0xC0330
#define PCI_DEVICE_AHCI 0x10601
#define PCI_DEVICE_BGA 0x30000
#define PCI_DEVICE_HOST_BRIDGE 0x60000
#define PCI_DEVICE_ISA_BRIDGE 0x60100
#define PCI_DEVICE_ETHERNET 0x20000

#define PCI_DEVICE_XHCI_STR "xHCI"
#define PCI_DEVICE_AHCI_STR "aHCI"
#define PCI_DEVICE_BGA_STR "BGA"
#define PCI_DEVICE_ETHERNET_STR "Ethernet"
#define PCI_DEVICE_HOST_BRIDGE_STR "Host bridge"
#define PCI_DEVICE_ISA_BRIDGE_STR "ISA bridge"

//Vendor codes and strings
#define PCI_VENDOR_INTEL 0x8086
#define PCI_VENDOR_INTEL_STR "Intel"
#define PCI_VENDOR_BOCHS 0x1234
#define PCI_VENDOR_BOCHS_STR "Bochs"
#define PCI_VENDOR_REDHAT 0x1b36
#define PCI_VENDOR_REDHAT_STR "Redht"