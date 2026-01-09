#include <stdint.h>
#include <kcache.h>
#include <kmalloc.h>
#include <string.h>
#include <paging.h>
#include <kprint.h>
#include <ahci.h>
#include <cpu.h>
#include <pci.h>
#include <kernel.h>
#include <sleep.h>
#include <libhook.h>
#include <libipc.h>

kcache *portCache = NULL;
struct ahciInfo ahci;

kcacheConstructor portConstructor(void *mem){
    memset(mem,0x0,0);
    return mem;
}

void ahciInterrupt(registers r){

}

void initPort(uint16_t port){
    //Activate port
    ahci.mmio->port[port].command = AHCI_PORT_CMD_ICC(AHCI_PORT_CMD_ICC_ACTIVE);
    //Get structures
    uint64_t commandList = (uint64_t)kcacheAlloc(portCache,0);
    if(commandList == 0x0){kpanic("Failed to allocate command list",commandList);}
    kinfo("Command list allocated to phys 0x%lx\n",commandList);
    ahci.mmio->port[port].cmdListBaseLower = commandList;

}
struct pciMSIInfo ahciMSI;

/*!
    !D Initilizes AHCI
    !I bus: PCI bus AHCI resides on
    !I dev: PCI device number
    !R None
*/
void initAHCI(uint8_t bus, uint8_t dev){
    kinfo("Initilziing AHCI\n");
    //Setup PCI command register
    uint32_t cmd = PCI_HEADER_COMMAND(getPCIData(bus,dev,0,PCI_HEADER_STATUS_COMMAND));
    kinfo("Old CMD 0x%lx\n",(uint64_t)cmd);
    cmd = PCI_HEADER_COMMAND_MEMORY_SPACE|PCI_HEADER_COMMAND_ID|PCI_HEADER_COMMAND_BUS_MASTER|PCI_HEADER_COMMAND_IO_SPACE;
    kinfo("New CMD 0x%lx\n",(uint64_t)cmd);
    setPCIData(bus,dev,0,PCI_HEADER_STATUS_COMMAND,cmd);
    //Get AHCI's BAR
    uint32_t bar = (uint64_t)(getPCIData(bus,dev,0,AHCI_PCI_ABAR)&0xFFFFE000);
    if(bar == 0x0){kwarn("BAR is NULL. Assuming no drives\n"); return;}
    uint64_t virt = (uint64_t)vmallocPhys((uint64_t)bar,sizeof(struct ahciMMIO),VTYPE_MMIO,VFLAG_WRITE|VFLAG_UC|VFLAG_MAKE);
    KASSERT((virt != 0x0),"valloc() failed");
    kinfo("Mapped 0x%x->0x%lx\n",bar,virt);
    ahci.mmio = (struct ahciMMIO*)virt;
    ahci.physBAR = bar;
    //Init AHCI
    //Save PI and capabilities
    uint32_t ver = ahci.mmio->version;
    uint32_t pi = ahci.mmio->portsImplimented;
    uint32_t cap = ahci.mmio->hostCapabilities;
    kinfo("[%lx.%lx|0x%lx PI|0x%lx CAP]\n",(uint64_t)AHCI_VERSION_MAJOR(ver),(uint64_t)AHCI_VERSION_MINOR(ver),(uint64_t)pi,(uint64_t)cap);
    //Check AHCI mode
    uint32_t ae = ahci.mmio->globalHostControl;
    kinfo("AHCI: ");
    if(ae&AHCI_GLOBAL_HBA_CONTROL_AE){knone("Enabled\n");}
    else{
        //Enable AHCI mode
        knone("Disabled\n");
        ahci.mmio->globalHostControl = ae|AHCI_GLOBAL_HBA_CONTROL_AE;
    }
    //Do BIOS to OS handoff if supported
    if(ahci.mmio->exCapabilities&AHCI_xHOST_CAPABILITIES_BOH){
        kinfo("Performing BIOS to OS handoff\n");
        ahci.mmio->biosHandoff = AHCI_BIOS_HANDOFF_CS_OOS;
        while(ahci.mmio->biosHandoff&AHCI_BIOS_HANDOFF_CS_BB){
            sysSleep(10);
        }
        kinfo("OS now owns the HBA\n");
    }
    //Reset HBA
    kinfo("Resetting HBA\n");
    ahci.mmio->globalHostControl = AHCI_GLOBAL_HBA_CONTROL_HR;
    //Wait for host reset to be completed
    for(;;){
        sysSleep(32);
        if(ahci.mmio->globalHostControl&AHCI_GLOBAL_HBA_CONTROL_HR){
            continue;
        }
        break;
    }
    kinfo("Reset completed\n");
    //Enable AHCI mode
    ahci.mmio->globalHostControl |= AHCI_GLOBAL_HBA_CONTROL_AE;
    //Create cache for port structures
    portCache = kcacheCreate("ahci_port_cache",0x400,0x400,MEMORY_TYPE_LOW,0,NULL,NULL);
    KASSERT((portCache != NULL),"NULL portCache");
    //Init interrupts
    bool msi = initMSIX(&ahciMSI,bus,dev,0);
    if(!msi){
        //Allocate interrupt vector
        uint16_t vec = allocateInterrupt(ahciInterrupt);
        KASSERT((vec != 0),"Failed to allocate an interrupt");
        //Write vector to PCI space
        setPCIData(bus,dev,0,PCI_HEADER_INT,(uint32_t)vec);
    }
    else{enableMSIX(&ahciMSI);}
    uint32_t cslots = AHCI_HBA_CAPABILITIES_NCS(cap);
    //Check ports and enable their interrupts
    kinfo("Checking ports for drives\n");
    uint32_t maxPorts = AHCI_HBA_CAPABILITIES_NPS&cap;
    for(uint32_t i = 0; i < 32; ++i){
        if((pi&(1<<i)) == 0x0){continue;}
        uint32_t prt = ahci.mmio->port[i].sataStatus;
        if(AHCI_PORT_SSTS_DET(prt) == AHCI_PORT_SSTS_DET_NONE){continue;}
        knone("Device on port 0x%x\n",i);
        initPort(i);
    }
    kinfo("Intilized ports\n");
}