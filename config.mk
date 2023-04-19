#Emulators
x86_64_EMU = qemu-system-x86_64
AARCH64_EMU = qemu-system-aarch64
KVM_EMU = kvm
ARM_MACHINE = raspi3b
#Compilers
C_CROSS_COMPILER = x86_64-elf-gcc
C_COMPILER = gcc
x64_AS = nasm
A64_AS = as
#Linkers
x64_LD = ld
A64_LD = arm-none-eabi-ld
#Paths
GNU_EFI_REPO = https://github.com/vathpela/gnu-efi
OUTPUT_PATH = ./Output
OVMF_PATH = /usr/share/qemu
EFI_PATH = ./Bootloader/Resources
OTHER_PATH = ./Other
HDD_PATH = /media/rover/6297-2783
#Other
EXPERIMENTAL_ARGS = 