#Emulators
x86_64_EMU = qemu-system-x86_64
AARCH64_EMU = qemu-system-aarch64
ARM_MACHINE = raspi3b
#Compilers
#For compiling the kernel/bootloader C code
CROSS_COMPILER = ~/opt/cross/bin/x86_64-elf-gcc
#For compiling hosted C code
C_COMPILER = gcc
x64_AS = nasm
A64_AS = as
#Linkers
x64_LD = ld
A64_LD = arm-none-eabi-ld 
#Paths
GNUEFI_REPO = https://github.com/vathpela/gnu-efi
OUTPUT_PATH = ./Output
OVMF_PATH = /usr/share/OVMF/x64
EFI_PATH = ./Bootloader/Resources
OTHER_PATH = ./Other
HDD_PATH = /mnt/1A3C-AD55
USB_PATH = /run/media/rover/CAF4-80D1
#Other
EXPERIMENTAL_ARGS =