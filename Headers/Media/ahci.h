#pragma once

#define AHCI_PCI_HBA 0x0
#define AHCI_PCI_CMD 0x4
#define AHCI_PCI_STS 0x6
#define AHCI_PCI_RID 0x8
#define AHCI_PCI_CC 0x9
#define AHCI_PCI_CLS 0xC
#define AHCI_PCI_MLT 0xD
#define AHCI_PCI_HTYPE 0xE
#define AHCI_PCI_BIST 0x0F
#define AHCI_PCI_ABAR 0x24
#define AHCI_PCI_SS 0x2C
#define AHCI_PCI_EROM 0x30
#define AHCI_PCI_CAP 0x34
#define AHCI_PCI_INTR 0x3C
#define AHCI_PCI_MGNT 0x3E
#define AHCI_PCI_MLAT 0x3F

#define AHCI_PCI_CMD_BUS_MASTER_ENABLE 1<<2

#define AHCI_HOST_CAPABILITIES 0x0 //HBA capabilites
#define AHCI_GLOBAL_HOST_CONTROL 0x4 //Control bits for all HBAs
#define AHCI_INTERRUPT_STATUS 0x8 //Port interrupt status
#define AHCI_PORTS_IMPLEMENTED 0xC //Ports available for the OS to use
#define AHCI_VERSION 0x10 //Version of AHCI the controller is compliant with
#define AHCI_COMMAND_CC_CONTROL 0x14 //Configuration register of command completion coalescing
#define AHCI_COMMAND_CC_PORTS 0x18 //Ports that are coalesced
#define AHCI_EM_LOCATION 0x1C //Offset/Sz of Enclosure management
#define AHCI_EM_CONTROL 0x20 //Control bits for the enclosure management
#define AHCI_xHOST_CAPABILITIES 0x24 //Extended HBA capabilities
#define AHCI_BIOS_HANDOFF_CS 0x28 //BOH/Control register for who owns the HBAs

#define AHCI_PORT_CONTROL(port) (0x100+(port*0x80)) //Offset for specified port number

#define AHCI_PORT_CLBL 0x0 //Command list lower
#define AHCI_PORT_CLBU 0x4 //Command list upper
#define AHCI_PORT_FBL 0x8 //FIS base lower
#define AHCI_PORT_FBU 0xC //FIS base upper
#define AHCI_PORT_IS 0x10 //Interrupt status
#define AHCI_PORT_IE 0x14 //Enable interrupts
#define AHCI_PORT_CMD 0x18 //Port command and status
#define AHCI_PORT_TFD 0x20 //Task file data
#define AHCI_PORT_SIG 0x24 //Signature
#define AHCI_PORT_SSTS 0x28 //SATA Status
#define AHCI_PORT_SCTL 0x2C //SATA Control
#define AHCI_PORT_SERR 0x30 //SATA Error
#define AHCI_PORT_SACT 0x34 //SATA Active
#define AHCI_PORT_CI 0x38 //Command issue
#define AHCI_PORT_NTF 0x4C //SATA Notification
#define AHCI_PORT_FBS 0x50 //FIS base switching
#define AHCI_PORT_DEVSLP 0x54 //DEVSLP
#define AHCI_PORT_VS 0x70 //Vendor specific

#define AHCI_PORT_CMD_ST 1<<0 //Start/stop
#define AHCI_PORT_CMD_SUD 1<<1 //Spin up device
#define AHCI_PORT_CMD_POD 1<<2 //Power on device
#define AHCI_PORT_CMD_CLO 1<<3 //Command list override
#define AHCI_PORT_CMD_FRE 1<<4 //FIS receive enable
#define AHCI_PORT_CMD_CCS(cmd) ((cmd>>8)&0x0F) //Current command slot
#define AHCI_PORT_CMD_MPSS 1<<13 //Mechanial presence switch
#define AHCI_PORT_CMD_FR 1<<14 //FIS recieve running
#define AHCI_PORT_CMD_CR 1<<15 //Command running
#define AHCI_PORT_CMD_CPS 1<<16 //Cold presence state
#define AHCI_PORT_CMD_PMA 1<<17 //Port multiplier attached
#define AHCI_PORT_CMD_HPCP 1<<18 //Hot plug capable port
#define AHCI_PORT_CMD_MPSP 1<<19 //Mechanical presence switch attached to port
#define AHCI_PORT_CMD_CPD 1<<20 //Cold presence detection
#define AHCI_PORT_CMD_ESP 1<<21 //External sata port
#define AHCI_PORT_CMD_FBSCP 1<<22 //FIS-Based switching supported
#define AHCI_PORT_CMD_APSTE 1<<23 //Automatic partial to slumber enable
#define AHCI_PORT_CMD_ATAPI 1<<24 //Device is ATAPI
#define AHCI_PORT_CMD_DLAE 1<<25 //Drive LED on ATAPI enable
#define AHCI_PORT_CMD_ALPE 1<<26 //Aggressive link power enable
#define AHCI_PORT_CMD_ASP 1<<27 //Aggressive slumber/partial
#define AHCI_PORT_CMD_ICC(cmd) (cmd>>28) //Interface communication control

#define AHCI_PORT_CMD_ICC_NOOP 0x0
#define AHCI_PORT_CMD_ICC_ACTIVE 0x1
#define AHCI_PORT_CMD_ICC_PARTIAL 0x2
#define AHCI_PORT_CMD_ICC_SLUMBER 0x6
#define AHCI_PORT_CMD_ICC_DEVSLP 0x8

#define AHCI_PORT_SSTS_DET(STS) (STS&0x0F) //Detection status
#define AHCI_PORT_SSTS_SPD(STS) ((STS>>4)&0x0F) //Speed status
#define AHCI_PORT_SSTS_IPM(STS) ((STS>>8)&0x0F) //Power management
#define AHCI_PORT_SSTS_DET_NONE 0x0 //No device detected
#define AHCI_PORT_SSTS_DET_NPHYS 0x1 //Present with no physical communication
#define AHCI_PORT_SSTS_DET_PRES 0x2 //Present with phys communication
#define AHCI_PORT_SSTS_DET_OFFLINE 0x3 //Offline
#define AHCI_PORT_SSTS_SPD_NONE 0x0 //No device
#define AHCI_PORT_SSTS_SPD_GEN1 0x1 //Gen 1 speed
#define AHCI_PORT_SSTS_SPD_GEN2 0x2 //Gen 2 speed
#define AHCI_PORT_SSTS_SPD_GEN3 0x3 //Gen 3 speed
#define AHCI_PORT_SSTS_IPM_NONE 0x0 //No device
#define AHCI_PORT_SSTS_IPM_ACTIVE 0x1 //Device active
#define AHCI_PORT_SSTS_IPM_PARTIAL 0x2 //Partially active
#define AHCI_PORT_SSTS_IPM_SLUMBER 0x3 //Device asleep
#define AHCI_PORT_SSTS_IPM_DEVSLP 0x4 //Device in deep sleep

#define AHCI_PORT_SCTL_DET_NO_ACTION 0x0
#define AHCI_PORT_SCTL_DET_COMRESET 0x1
#define AHCI_PORT_SCTL_DET_DISABLE 0x4
#define AHCI_PORT_SCTL_SPD_NO_LIMIT 0x0
#define AHCI_PORT_SCTL_SPD_GEN1 0x1<<4
#define AHCI_PORT_SCTL_SPD_GEN2 0x2<<4
#define AHCI_PORT_SCTL_SPD_GEN3 0x3<<4
#define AHCI_PORT_SCTL_IPM_NO_LIMIT 0x0

#define AHCI_HOST_CAPABILITIES_NPS 0x0F //Number of ports
#define AHCI_HOST_CAPABILITIES_SXS 1<<5 //External sata support
#define AHCI_HOST_CAPABILITIES_EMS 1<<6 //Enclosure managenment supported
#define AHCI_HOST_CAPABILITIES_CCCS 1<<7 //COMMAND_CC support
#define AHCI_HOST_CAPABILITIES_NCS(HC) ((HC>>8)&0x0F) //Number of command slots
#define AHCI_HOST_CAPABILITIES_PSC 1<<13 //Partial state capable
#define AHCI_HOST_CAPABILITIES_SSC 1<<14 //Slumber state capable
#define AHCI_HOST_CAPABILITIES_PMD 1<<15 //multiple DRQ block support
#define AHCI_HOST_CAPABILITIES_FBSS 1<<16 //port mul FIS switching
#define AHCI_HOST_CAPABILITIES_SPM 1<<17 //port mul supported
#define AHCI_HOST_CAPABILITIES_SAM 1<<18 //AHCI only
#define AHCI_HOST_CAPABILITIES_ISS(HC) ((hc>>20)&0x0F) //Interface speed
#define AHCI_HOST_CAPABILITIES_ISS_GEN1 0x1 //1.5 Gbps
#define AHCI_HOST_CAPABILITIES_ISS_GEN2 0x2 //3 Gbps
#define AHCI_HOST_CAPABILITIES_ISS_GEN3 0x3 //6 Gbps
#define AHCI_HOST_CAPABILITIES_SCLO 1<<24 //PxCMD.SLO support
#define AHCI_HOST_CAPABILITIES_SAL 1<<25 //Activity light supported
#define AHCI_HOST_CAPABILITIES_SALP 1<<26 //Auto-link gen supported
#define AHCI_HOST_CAPABILITIES_SSS 1<<27 //Staggered spinup supported
#define AHCI_HOST_CAPABILITIES_SMPS 1<<28 //Mechanical switches
#define AHCI_HOST_CAPABILITIES_SSNTF 1<<29 //SNotification register support
#define AHCI_HOST_CAPABILITIES_SNCQ 1<<30 //Native command queueing support
#define AHCI_HOST_CAPABILITIES_S64A 1<<31 //64Bit structure support

#define AHCI_GLOBAL_HOST_CONTROL_HR 1<<0 //Interal reset of HBA
#define AHCI_GLOBAL_HOST_CONTROL_IE 1<<1 //Interrupt enable
#define AHCI_GLOBAL_HOST_CONTROL_MRSM 1<<2 //If set, HBA reverted to only one vector
#define AHCI_GLOBAL_HOST_CONTROL_AE 1<<31 //Enable AHCI

#define AHCI_INTERRUPT_STATUS_OF_PORT(port,ints) ((ints)&((uint32)1<<port))

#define AHCI_VERSION_MAJOR(vs) (vs>>16)
#define AHCI_VERSION_MINOR(vs) (vs&0xFFFF)

#define AHCI_COMMAND_CC_CONTROL_EN 1<<0 //Enable Command Completion Coalesing
#define AHCI_COMMAND_CC_CONTROL_INT(CCC) ((CCC>>3)&0x0F) //Interrupt vector used
#define AHCI_COMMAND_CC_CONTROL_CC_GET(CCC) ((CCC>>8)&0x7F) //Command completions per int
#define AHCI_COMMAND_CC_CONTROL_CC_SET(CCC,n) (((n&0x7F)<<8)|CCC) //Set CC
#define AHCI_COMMAND_CC_CONTROL_CC_CLR(CCC) ((AHCI_COMMAND_CC_CONTROL_CC_GET(CCC)<<8)^CCC) //Clear CC to 0x0
#define AHCI_COMMAND_CC_CONTROL_TV_GET(CCC) (CCC>>16) //Timeout value in MS
#define AHCI_COMMAND_CC_CONTROL_TV_SET(CCC,v) (((uint32)v<<16)|CCC) //Set TV

#define AHCI_EM_LOCATION_SZ(EML) (EML&0xFFFF) //Size of buffer
#define AHCI_EM_LOCATION_OFST(EML) (EML>>16) //Offset, in dwords, to buffer relative to the start of the AHCI bar

#define AHCI_EM_CONTROL_MR 1<<0 //Message recieved into message buffer
#define AHCI_EM_CONTROL_TM 1<<8 //Transmits message when set
#define AHCI_EM_CONTROL_RST 1<<9 //Resets all enclosures when set
#define AHCI_EM_CONTROL_LED 1<<16 //LED message supported
#define AHCI_EM_CONTROL_SAFTE 1<<17 //SAF-TE supported
#define AHCI_EM_CONTROL_SES2 1<<18 //SES-2 supported
#define AHCI_EM_CONTROL_SGPIO 1<<19 //SGPIO supported
#define AHCI_EM_CONTROL_SMB 1<<24 //One shared message buffer
#define AHCI_EM_CONTROL_XMT 1<<25 //Message transmit only
#define AHCI_EM_CONTROL_ALHD 1<<26 //Activity LED hardware driven
#define AHCI_EM_CONTROL_PM 1<<27 //Port multiplier supported

#define AHCI_xHOST_CAPABILITIES_BOH 1<<0 //BIOS/OS handoff supported
#define AHCI_xHOST_CAPABILITIES_NVMHCI 1<<1 //NVMHCI supported
#define AHCI_xHOST_CAPABILITIES_APST 1<<2 //Automatic partial to slumber supported
#define AHCI_xHOST_CAPABILITIES_SDS 1<<3 //Sleep device supported
#define AHCI_xHOST_CAPABILITIES_SADM 1<<4 //Agressive sleep device supported
#define AHCI_xHOST_CAPABILITIES_DESO 1<<5 //Device may only use DEVSLP when interface is in slumber

#define AHCI_BIOS_HANDOFF_CS_BOS 1<<0 //Bios owns HBA
#define AHCI_BIOS_HANDOFF_CS_OOS 1<<1 //OS owns HBA
#define AHCI_BIOS_HANDOFF_CS_SMI 1<<2 //SMI when OOS is set to 1
#define AHCI_BIOS_HANDOFF_CS_OOC 1<<3 //OS ownership change
#define AHCI_BIOS_HANDOFF_CS_BB 1<<4 //BIOS is busy cleaning up for ownership change