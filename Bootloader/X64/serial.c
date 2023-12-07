#include <efi.h>
#include <efilib.h>
#include "boot.h"
#include "bootPaging.h"
#include "mmap.h"
#include "bootMmap.h"

//Checks if a serial port is available and initilizes it
EFI_STATUS initSerial(){
    EFI_GUID serialGuid = EFI_SERIAL_IO_PROTOCOL_GUID;
    EFI_SERIAL_IO_PROTOCOL *serial = NULL;
    EFI_STATUS status = LibLocateProtocol(&serialGuid,(void**)&serial);
    if(EFI_ERROR(status)){Print(L"SERIAL: No serial is available or locating failed\n"); return status;}
    status = !EFI_SUCCESS;
    Print(L"SERIAL: Located serial IO\n");
    if(uefi_call_wrapper(serial->Reset,1,serial) != EFI_SUCCESS){Print(L"SERIAL: Failed to reset device\n"); return status;}
    Print(L"SERIAL: Reset device\n");
    status = uefi_call_wrapper(serial->SetAttributes,7,serial,0,0,0,NoParity,8,DefaultStopBits);
    if(status == EFI_DEVICE_ERROR){Print(L"SERIAL: Device error when setting attributes\n"); return status;}
    if(status == EFI_INVALID_PARAMETER){Print(L"SERIAL: Invalid parameter when setting attributes\n"); return status;}
    Print(L"SERIAL: Writing string to serial\n");
    const CHAR16 *str = L"SERIAL: TEST\n";
    if(uefi_call_wrapper(serial->Write,3,serial,14,(void*)str) != EFI_SUCCESS){Print(L"SERIAL: Failed to write to serial port\n"); return status;}
    Print(L"SERIAL: Write succeded\n");
    return EFI_SUCCESS;
}