#=====Configurable=====#
#=Options
USE_HDD = 0
USE_KVM = 0
USE_GDB = 0
#=Paths
GNU_EFI_PATH = /home/rover/Documents/gnu-efi
HDD_PATH = /media/rover/6297-2783
EFI_PATH = ./Boot/Resources
OTHER_PATH = ./Resources
OVMF_PATH = /usr/share/qemu
OUTPUT = ./Output
#=Qemu
QARGS = -bios ${OVMF_PATH}/OVMF.fd
QARGS += -m 2G
QARGS += -d cpu_reset
QARGS += -D ${OTHER_PATH}/qemu.log
QARGS += -cdrom ./RoverOS.iso
QARGS += -serial file:${OTHER_PATH}/RoverOS.log
QARGS += -monitor stdio
QARGS += -usb
QARGS += -device ahci,id=ahci
QARGS += -drive format=raw,file=hdd.img,id=hddi
QARGS += -rtc base=utc
QARGS += -no-reboot
ifeq (${USE_GDB}, 1)
QARGS += -s -S
endif
ifeq (${USE_KVM}, 1)
EMULATOR = kvm
else
EMULATOR = qemu-system-x86_64
endif

#===Non=Configurable===#
#=GCC
ifeq ("$(wildcard $("$$HOME/opt/cross/bin/x86_64-elf-gcc"))","")
	CC = "$$HOME/opt/cross/bin/x86_64-elf-gcc"
else
	CC = gcc
endif
EFI_CC = ${CC}
#=ASM
AS = nasm
AS_FLAGS = -g -Fdwarf -felf64
#=Flags
CFLAGS = -Wno-format -Wno-unused-variable -Wno-incompatible-pointer-types -Wno-unused-parameter -Wno-implicit-function-declaration -ffreestanding -mcmodel=large -m64 -fno-pic -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -c -ggdb
EFI_CFLAGS = -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -Wno-packed-bitfield-compat -c
LD_FLAGS = -m elf_x86_64 -nostdlib -T ${OTHER_PATH}/linker.ld
EFI_LD_FLAGS = -shared -Bsymbolic -L${EFI_PATH}/lib -L${EFI_PATH}/gnuefi -T${EFI_PATH}/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/gnuefi/crt0-efi-x86_64.o
#=Include
INCLUDE_0 := $(shell cd ./Headers && find . -type d)
#INCLUDE := $(subst ./,-I./Headers/,$(INCLUDE_0))
INCLUDE = -I./Headers/IO \
		  -I./Headers/Memory \
		  -I./Headers/System \
		  -I./Headers/libc/std

EFI_INCLUDE = -I${EFI_PATH}/Include -I./Boot/Include $(INCLUDE)
#=Files
CS_0 := $(shell find . -path ./Boot -prune -o -path ./libc -prune -o -name *.c)
CS_1 := $(subst ./Boot, ,$(CS_0))
CS_2 := $(subst ./libc, ,$(CS_1))
CF := $(subst .c,.o,$(CS_2))
EF_0 := $(shell find . -path ./Kernel -prune -o -path ./libc -prune -o -name *.c)
EF_1 := $(subst ./Kernel, ,$(EF_0))
EF_2 := $(subst ./libc, ,$(EF_1))
EF := $(subst .c,_efi.o,$(EF_2))
#ASM
AS_0 := $(shell find . -path ./Kernel -prune -o -path ./libc -prune -o -name *.asm)
AS_1 := $(subst ./Kernel, ,$(AS_0))
AS_2 := $(subst ./libc, ,$(AS_1))
AF := $(subst .asm,_asm.o,$(AS_2))
#=Rules
#ASM
%_asm.o: $(notdir %.asm)
	${AS} ${AS_FLAGS} ./$< -o ${OUTPUT}/$(subst Assembly/,,$@)
#C
%.o: $(notdir %.c)
	${CC} ${CFLAGS} ${INCLUDE} ./$< -o ${OUTPUT}/$(subst Kernel/,,$@)
%_efi.o: $(notdir %.c)
	${EFI_CC} ${EFI_CFLAGS} ${EFI_INCLUDE} ./$< -o ${OUTPUT}/$@
#=Linker
LINKER = ld
EFI_LINKER = ${LINKER}

all: efi $(CF) mov compile iso hdd hdw clean
	${EMULATOR} ${QARGS}

#EFI

efi: $(EF) $(AF)
	cp ${EFI_PATH}/lib/data.o ${OUTPUT}/Boot/data.o
	${EFI_LINKER} ${EFI_LD_FLAGS} ${OUTPUT}/Boot/*.o -o ${OUTPUT}/Boot/boot.so -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 ${OUTPUT}/Boot/boot.so ${OUTPUT}/Boot/RoverOS.efi
	objcopy --only-keep-debug ${OUTPUT}/Boot/boot.so ${OTHER_PATH}/Bootloader.sym

hdw:
    ifeq (${USE_HDD}, 1)
	mkdir -p ./HDD/Binaries
	cp ${OUTPUT}/Boot/RoverOS.efi ./HDD/Binaries/RoverOS.efi
	cp ${OUTPUT}/Boot/RoverOS.efi ./HDD/RoverOS.efi
	cp ${OUTPUT}/RoverOS.bin ./HDD/Binaries/RoverOS.bin
	cp ${OUTPUT}/RoverOS.bin ./HDD/RoverOS.bin
	cp -r ./HDD/* ${HDD_PATH}
    endif

iso:
	mkdir -p ./ISO/boot/grub
	cp ${OTHER_PATH}/grub.cfg ./ISO/boot/grub
	grub-mkrescue -o ./RoverOS.iso ./ISO
	rm -r ./ISO

#Kernel

mov:
	$(shell find ./Kernel/ -name *.o -exec mv {} ${OUTPUT} \;)

compile:
	${LD} ${LD_FLAGS} ${OUTPUT}/*.o -o ${OUTPUT}/RoverOS.bin
	${LD} ${LD_FLAGS} ${OUTPUT}/*.o -o ${OUTPUT}/RoverOS.o
	objcopy --only-keep-debug ${OUTPUT}/RoverOS.o ${OTHER_PATH}/RoverOS.sym

#Tools

required: dir
	mkdir ${EFI_PATH}/Include
	cp ${GNU_EFI_PATH}/inc/*.h ${EFI_PATH}/Include
	cp -r ${GNU_EFI_PATH}/inc/x86_64 ${EFI_PATH}/Include
	cp -r ${GNU_EFI_PATH}/x86_64/lib ${EFI_PATH}
	cp -r ${GNU_EFI_PATH}/x86_64/gnuefi ${EFI_PATH}
	cp ${GNU_EFI_PATH}/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/gnuefi/elf_x86_64_efi.lds

clean:
	$(shell rm -r ${OUTPUT}/*.o)
	$(shell rm -r ${OUTPUT}/Boot/*.o)
	$(shell rm -r ${OUTPUT}/Boot/*.so)

hdd:
	dd if=/dev/zero of=hdd.img bs=1M count=50
	mkfs.vfat -F 32 hdd.img
	mmd -i hdd.img ::/Binaries
	mmd -i hdd.img ::/EFI
	mcopy -i hdd.img ${OUTPUT}/RoverOS.bin ::/Binaries/
	mcopy -i hdd.img ${OUTPUT}/RoverOS.bin ::/
	mcopy -i hdd.img ${OUTPUT}/Boot/RoverOS.efi ::/EFI/

reset: mov clean
	if test -d ${EFI_PATH}; then rm -r ${EFI_PATH}; fi
	if test -d ${OUTPUT}; then rm -r ${OUTPUT}; fi
	if test ${OTHER_PATH}/RoverOS.sym; then rm ${OTHER_PATH}/RoverOS.sym; fi
	if test ${OTHER_PATH}/Bootloader.sym; then rm ${OTHER_PATH}/Bootloader.sym; fi
	if test ${OTHER_PATH}/RoverOS.log; then rm ${OTHER_PATH}/RoverOS.log; fi
	if test ${OTHER_PATH}/qemu.log; then rm ${OTHER_PATH}/qemu.log; fi
	if test ./hdd.img; then rm ./hdd.img; fi
	if test -d ./ISO; then rm -r ./ISO; fi
	if test -d ./HDD; then rm -r ./HDD; fi
	if test ./RoverOS.iso; then rm ./RoverOS.iso; fi

dir:
	if ! test -d ${EFI_PATH}; then mkdir ${EFI_PATH}; fi
	if ! test -d ${OUTPUT}/Boot; then mkdir -p ${OUTPUT}/Boot; fi

.PHONY: required clean reset mov hdw