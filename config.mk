#Emulators
X64_EMU = qemu-system-x86_64
A64_EMU = qemu-system-aarch64
A64_MACHINE = raspi4b
#Compilers
#For compiling the kernel/bootloader C code
X64_COMPILER = gcc
A64_COMPILER = gcc
#For compiling hosted C code
C_COMPILER = gcc
#For compiling assembly
X64_AS = nasm
A64_AS = arm-none-eabi-as
#Linkers
X64_LD = ld
A64_LD = arm-none-eabi-ld 
#Paths
EFI_REPO = https://github.com/Roverx64/SEFI
OUTPUT_PATH = ./Output
OVMF_PATH = ./Other
EFI_PATH = ./Bootloader/SEFI
OTHER_PATH = ./Other
HDD_PATH = /mnt/HDD
USB_PATH = /run/media/rover/USB
RAMDISK_ROOT = ./Ramdisk/FS
#Other
EXPERIMENTAL_ARGS = -d int,guest_errors,cpu_reset