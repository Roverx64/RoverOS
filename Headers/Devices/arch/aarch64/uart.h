#pragma once
#include <stdint.h>

//BCM2711 UART

#define UART_0_BASE 0x7E201000
//UART1 wasn't in the spec, so it is left out
#define UART_2_BASE 0x7E201400
#define UART_3_BASE 0x7E201600
#define UART_4_BASE 0x7E201800
#define UART_5_BASE 0x7E201a00

#define UART_DR_DATA_OUT(b) (b&0xFF)
#define UART_DR_DATA_INT(i) (i&0xFF)
#define UART_DR_FE 1<<8 //Framing error
#define UART_DR_PE 1<<9 //Parity error
#define UART_DR_BE 1<<10 //Break error
#define UART_DR_OE 1<<11 //Overrun error

#define UART_RSRECR_FE 1<<0 //Framing error
#define UART_RSRECR_PE 1<<1 //Parity error
#define UART_RSRECR_BE 1<<2 //Break error
#define UART_RSRECR_OE 1<<3 //Overrun error

#define UART_FE_CTS 1<<0 //Clear to send
#define UART_FE_DSR 1<<1 //Ignored
#define UART_FE_DCD 1<<2 //Ignored
#define UART_FE_BUSY 1<<3 //RO
#define UART_FE_RXFE 1<<4 //Set when recieve is empty
#define UART_FE_TXFF 1<<5 //Set when transmit is full
#define UART_FE_RXFF 1<<6 //Set when recieve is full
#define UART_FE_TXFE 1<<7 //Set when transmit is full
#define UART_FE_RI 1<<8 //Ignored

#define UART_LCRH_BRK 1<<0 //Send break
#define UART_LCRH_PEN 1<<1 //Parity enable
#define UART_LCRH_EPS 1<<2 //Even parity enable
#define UART_LCRH_STP2 1<<3 //Two stop bits
#define UART_LCRH_FEN 1<<4 //Enable FIFOs
#define UART_LCRH_WLEN(w) (((uint32_t)w&0x03)<<5) //Word length
#define UART_LCRH_SPS 1<<7 //Sticky parity

#define UART_CR_UARTEN 1<<0 //UART enable
#define UART_CR_SIREN 1<<1 //Ignored
#define UART_CR_SIRLP 1<<2 //Ignored
#define UART_CR_LBE 1<<7 //Loopback enable
#define UART_CR_TXE 1<<8 //Transmit enable
#define UART_CR_RXE 1<<9 //Recieve enable
#define UART_CR_DTR 1<<10 //Ignored
#define UART_CR_RTS 1<<11 //Request to send
#define UART_CR_OUT1 1<<12 //Ignored
#define UART_CR_OUT2 1<<13 //Ignored
#define UART_CR_RTSEN 1<<14 //RTS hardware control flow
#define UART_CR_CTSEN 1<<15 //CTS hardware control flow

#define UART_IFLS_TXIFLSEL(l) (l&0x3) //Transmit level select
#define UART_IFLS_RXIFLSEL(l) (((uint32_t)l&0x3)<<3) //Recieve level select

#define UART_IMSC_RIMM 1<<0 //Ignored
#define UART_IMSC_CTSMIM 1<<1 //Modem interrupt mask
#define UART_IMSC_DCDMIM 1<<2 //Ignored
#define UART_IMSC_DSRMIM 1<<3 //Ignored
#define UART_IMSC_RXIM 1<<4 //Recieve interrupt mask
#define UART_IMSC_TXIM 1<<5 //Transmit interrupt mask
#define UART_IMSC_RTIM 1<<6 //Recieve timeout mask
#define UART_IMSC_FEIM 1<<7 //Framing error mask
#define UART_IMSC_PEIM 1<<8 //Parity error mask
#define UART_IMSC_BEIM 1<<9 //Break error mask
#define UART_IMSC_OEIM 1<<10 //Overrun mask

#define UART_RIS_RIRMIS 1<<0 //Ignored
#define UART_RIS_CTSRMIS 1<<1 //Modem int status
#define UART_RIS_DCDRMIS 1<<2 //Ingored
#define UART_RIS_DSRRMIS 1<<3 //Ignored
#define UART_RIS_RXRIS 1<<4 //Recieve int status
#define UART_RIS_TXRIS 1<<6 //Transmit int status
#define UART_RIS_RTRIS 1<<7 //Recieve timeout int status
#define UART_RIS_FERIS 1<<8 //Framing error int status
#define UART_RIS_PERIS 1<<9 //Parity error int status
#define UART_RIS_BERIS 1<<10 //Break error int status
#define UART_RIS_OERIS 1<<11 //Overrun error int status

#define UART_MIS_RIMMIS 1<<0 //Ignored
#define UART_MIS_CTSMIS 1<<1 //Modem int status
#define UART_MIS_DCDMMIS 1<<2 //Ingored
#define UART_MIS_DSRMMIS 1<<3 //Ignored
#define UART_MIS_RXMIS 1<<4 //Recieve int status
#define UART_MIS_TXMIS 1<<6 //Transmit int status
#define UART_MIS_RTMIS 1<<7 //Recieve timeout int status
#define UART_MIS_FEMIS 1<<8 //Framing error int status
#define UART_MIS_PEMIS 1<<9 //Parity error int status
#define UART_MIS_BEMIS 1<<10 //Break error int status
#define UART_MIS_OEMIS 1<<11 //Overrun error int status

#define UART_ICR_RIMIC 1<<0 //Ignored
#define UART_ICR_CTSIC 1<<1 //Modem int clear
#define UART_ICR_DCDMIC 1<<2 //Ingored
#define UART_ICR_DSRMIC 1<<3 //Ignored
#define UART_ICR_RXIC 1<<4 //Recieve int clear
#define UART_ICR_TXIC 1<<6 //Transmit int clear
#define UART_ICR_RTIC 1<<7 //Recieve timeout int clear
#define UART_ICR_FEIC 1<<8 //Framing error int clear
#define UART_ICR_PEIC 1<<9 //Parity error int clear
#define UART_ICR_BEIC 1<<10 //Break error int clear
#define UART_ICR_OEIC 1<<11 //Overrun error int clear

#define UART_DMA_RXDMAE 1<<0 //Recieve DMA enable
#define UART_DMA_TXDMAE 1<<1 //Transmit DMA enable
#define UART_DMA_DMAONERR 1<<2 //Disable DMA on error

struct uartMMIO{
    uint32_t data;
    uint32_t rsrecr;
    uint64_t pad; //Aligns members following rsrecr
    uint32_t flags;
    uint32_t ilpr; //not in use
    uint32_t intBRD; //Baud rate divisor
    uint32_t fracBRD; //Fractional baud rate divisor
    uint32_t lineControl;
    uint32_t control;
    uint32_t interruptFIFOLevelSelect;
    uint32_t interruptMaskSetClear;
    uint32_t rawInterruptStatus;
    uint32_t maskedInterruptStatus;
    uint32_t interruptClear;
    uint32_t dmaControl;
    uint32_t testControl;
    uint32_t integrationTestInput;
    uint32_t integrationTestOutput;
    uint32_t testData;
}__attribute__((packed));