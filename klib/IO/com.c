#include <stdint.h>
#include <com.h>
#include <port.h>

/*
Utilities for using the COM port
*/

//Checks if the COM port is able to take another byte
bool checkEmpty(uint16 port){
    uint8 r = inb(port+OFFSET_LSR);
    if(r&(0x1<<5)){return true;}
    return false;
}

/*This will be expanded to allow more flexibility*/
bool initCOMPort(uint16 port){
    //Set DLAB to enable baud write
    outb(port+OFFSET_LCR,0x80);
    //Set baud rate
    uint16 baud = DLV_DIVISOR/12;
    outb(port+OFFSET_DLV_HIGH,(uint8)(baud&0x00FF)); //Set low byte
    outb(port+OFFSET_DLV_LOW,(uint8)(baud&0xFF00)); //Set high byte
    //Set DLAB low to disable write & set LCR flags
    outb(port+OFFSET_LCR,0x83);
    //Set FIFO
    outb(port+OFFSET_FIFO,0x27);
    return true;
}

void writeByte(uint16 port, uint8 byte){
    while(checkEmpty(port) == false){}
    outb(port,byte);
}

void writeString(uint16 port, const char *str){
    for(int i = 0; str[i] != '\0'; ++i){
        writeByte(port,str[i]);
    }
}