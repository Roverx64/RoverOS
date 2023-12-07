#pragma once

#include <stdint.h>
#include "pci.h"

#define XHCI_PCI_BAR0 0x10
#define XHCI_PCI_BAR1 0x14
#define XHCI_PCI_CACHET 0x8
#define XHCI_PCI_CACHET_CACHESZ(ch) (ch&0xFF)
#define XHCI_PCI_CACHET_MAXLAT_TIMER(ch) ((ch>>8)&0xFF)
#define XHCI_PCI_CACHET_BIST(ch) ((ch>>24)&0xFF)
#define XHCI_PCI_CAPABILITIES 0x34
#define XHCI_PCI_CAPABILITIES_MASK 0xFF
#define XHCI_PCI_LATINT 0x3C
#define XHCI_PCI_LATINT_INTLINE(li) (li&0xFF)
#define XHCI_PCI_LATINT_INTPIN(li) ((li>>8)&0xFF)
#define XHCI_PCI_LATINT_MINGNT(li) ((li>>16)&0xFF)
#define XHCI_PCI_LATINT_MAXLAT(li) ((li>>24)&0xFF)
#define XHCI_PCI_DBRN 0x60
#define XHCI_PCI_DBRN_SBRN(db) (db&0xFF)
#define XHCI_PCI_DBRN_FLADJ(db) ((db>>8)&0xFF)
#define XHCI_PCI_DBRN_DBSEL(db) ((db>>16)&0xF)
#define XHCI_PCI_DBRN_DBSLD(db) ((db>>20)&0xF)

#define XHCI_CAPABILITIES_LENGTH 0x0 //Register length
#define XHCI_CAPABILITIES_HIVERSION 0x2 //Interface version
#define XHCI_CAPABILITIES_HCSPARAMS1 0x4 //Structure parameter 1
#define XHCI_CAPABILITIES_HCSPARAMS2 0x8 //Structure parameter 2
#define XHCI_CAPABILITIES_HCSPRARMS3 0xC //Structure parameter 3
#define XHCI_CAPABILITIES_HCCPARAMS1 0x10 //Capability parameter 1
#define XHCI_CAPABILITIES_DBOFF 0x14 //Doorbell offset
#define XHCI_CAPABILITIES_RTSOFF 0x18 //Runtine registers offset
#define XHCI_CAPABILITIES_HCCPARAMS2 0x1C //Capability parameter 2

#define XHCI_OPERATION_REGISTER_SPACE(bar,clen) ((void*)((uint64)bar+clen))
#define XHCI_OPERATION_REGISTER_USBCMD 0x0 //USB Command
#define XHCI_OPERATION_REGISTER_USBSTS 0x4 //USB Status
#define XHCI_OPERATION_REGISTER_PAGESIZE 0x8 //Page size 2^(n+12)
#define XHCI_OPERATION_REGISTER_DNCTRL 0x14 //Device notification control
#define XHCI_OPERATION_REGISTER_CRCR 0x18 //Command ring control
#define XHCI_OPERATION_REGISTER_DCBAAP 0x30 //Device context base address array pointer
#define XHCI_OPERATION_REGISTER_CONFIG 0x38
#define XHCI_OPERATION_REGISTER_PORTREG(prt) (0x400+(prt&0xC))
#define XHCI_OPERATION_REGISTER_PORTREG_PORTSC 0x0 //Port status and control
#define XHCI_OPERATION_REGISTER_PORTREG_PORTPMSC 0x4 //Port power management and control
#define XHCI_OPERATION_REGISTER_PORTREG_PORTLI 0x8 //Port link info
#define XHCI_OPERATION_REGISTER_PORTREG_PROTHLPMC 0xC //Port hardware lpm control

#define XHCI_OPERATION_REGISTER_USBCMD_RS 1<<0 //Run/stop bit 
#define XHCI_OPERATION_REGISTER_USBCMD_HCRST 1<<1 //Host controller reset
#define XHCI_OPERATION_REGISTER_USBCMD_INTE 1<<2 //Interrupt enable
#define XHCI_OPERATION_REGISTER_USBCMD_HSEE 1<<3 //Host system error enable
#define XHCI_OPERATION_REGISTER_USBCMD_LHCRST 1<<7 //Light HCRST. Resets host chip without affecting ports
#define XHCI_OPERATION_REGISTER_USBCMD_CSS 1<<8 //Controller save state
#define XHCI_OPERATION_REGISTER_USBCMD_CRS 1<<9 //Controller reset state
#define XHCI_OPERATION_REGISTER_USBCMD_EWE 1<<10 //Event wrap enable
#define XHCI_OPERATION_REGISTER_USBCMD_EU3S 1<<11 //
#define XHCI_OPERATION_REGISTER_USBCMD_CME 1<<13 //CEM enable

#define XHCI_OPERATION_REGISTER_USBSTS_CNR 1<<11 //Controller not ready

#define XHCI_OPERATION_REGISTER_CRCR_RCS 1<<0 //Ring cycle status
#define XHCI_OPERATION_REGISTER_CRCR_CS 1<<1 //Command stop
#define XHCI_OPERATION_REGISTER_CRCR_CA 1<<2 //Command abort
#define XHCI_OPERATION_REGISTER_CRCR_CRR 1<<3 //Command ring running
#define XHCI_OPERATION_REGISTER_CRCR_CRP(pnt) ((uint64)pnt<<6) //Command ring pointer

#define XHCI_COMMAND_TRB_ID_NORMAL 1
#define XHCI_COMMAND_TRB_ID_SETUP 2
#define XHCI_COMMAND_TRB_ID_DATA 3
#define XHCI_COMMAND_TRB_ID_STATUS 4
#define XHCI_COMMAND_TRB_ID_ISOCH 5
#define XHCI_COMMAND_TRB_ID_LINK 6
#define XHCI_COMMAND_TRB_ID_EVENT 7
#define XHCI_COMMAND_TRB_ID_NOOP 8 //No operation
#define XHCI_COMMAND_TRB_ID_ECMD 9 //Enable command slot
#define XHCI_COMMAND_TRB_ID_DCMD 10 //Disable command slot
#define XHCI_COMMAND_TRB_ID_ADCMD 11 //Address device command
#define XHCI_COMMAND_TRB_ID_CECMD 12 //Configure endpoint command

#define XHCI_COMMAND_NOOP(c,type) (((uint32)c|((uint32)type<<10)))