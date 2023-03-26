#=====Configurable=====#
#=Options
USE_GDB = 0
USE_HDD = 0
USE_KVM = 0
#=Target
#x86_64
#aarch64 (Not supported yet)
TARGET = x86_64
#=Paths
GNU_EFI_PATH = /path/to/gnu-efi
OUTPUT = ./Output
OVMF_PATH = /usr/share/qemu
EFI_PATH = ./Boot/Resources
OTHER_PATH = ./Other
HDD_PATH = /media/rover/6297-2783
#=Qemu
#x64
XARGS = -bios ${OVMF_PATH}/OVMF.fd -m 500M -d cpu_reset,int
XARGS += -D ${OTHER_PATH}/qemu.log -cdrom ./RoverOS.iso
XARGS += -serial file:${OTHER_PATH}/RoverOS.log
XARGS += -usb -device ahci,id=ahci
XARGS += -drive format=raw,file=hdd.img,id=hddi
XARGS += -rtc base=utc -monitor stdio
ifeq (${USE_GDB},1)
XARGS += -s -S
endif
#Arm
AMACHINE = raspi3b
AARGS = -M ${AMACHINE} -cpu cortex-a15 -kernel ${OUTPUT}/Compiled/armboot.elf
AARGS += -serial file:${OTHER_PATH}/RoverOS.log
#=Emulators
AEMU = qemu-system-aarch64
XEMU = qemu-system-x86_64
#===Non=Configurable===#
#Emu
ifeq (${TARGET},x86_64)
EMULATOR = ${XEMU}
QARGS = ${XARGS}
else ifeq (${TARGET},aarch64)
EMULATOR = ${AEMU}
QARGS = ${AARGS}
else
EMULATOR = ${KEMU}
QARGS = ${KARGS}
endif
ifeq (${USE_KVM},1)
EMULATOR = kvm
QARGS = ${XARGS}
endif
#=GCC
ifneq ("$(shell which x86_64-elf-gcc)","")
CC = x86_64-elf-gcc
else
CC = gcc
endif
#=ASM
ifeq (${TARGET},aarch64)
AS = arm-none-eabi-as
AS_FLAGS = -march=armv8-a -mcpu=cortex-a32
else
AS = nasm
XS_FLAGS = -g -Fdwarf -felf64
endif
#=Flags
#x64/KVM
XFLAGS = -mno-mmx -mcmodel=large -fno-pic -mno-sse -mno-sse2 -Wno-format -Wno-unused-variable -Wno-incompatible-pointer-types -Wno-unused-parameter -Wno-implicit-function-declaration -ffreestanding -m64 -mno-red-zone -c -ggdb
EFI_FLAGS = -Wno-implicit-int -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -Wno-packed-bitfield-compat -c
XLD_FLAGS = -m elf_x86_64 -nostdlib -T ${OTHER_PATH}/linker.ld -L./klib -L./libc
EFI_LD_FLAGS = -shared -Bsymbolic -L${EFI_PATH}/lib -L${EFI_PATH}/gnuefi -T${EFI_PATH}/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/gnuefi/crt0-efi-x86_64.o
#ARM
AFLAGS = 
ALD_FLAGS = -T ${OTHER_PATH}/armlinker.ld
#=Include
ifeq (${TARGET},x86_64)
INCLUDE := $(subst ./,-I./,$(wildcard ./Headers/*) $(wildcard ./Headers/libc/*) $(wildcard ./Headers/klib/*))
EFI_INCLUDE = -I${EFI_PATH}/Include -I./Boot/Include -I./Boot/Headers -I./Headers/System -I./Headers/Memory
else ifeq (${TARGET}, aarch64)
INCLUDE := $(subst ./,-I./,$(wildcard ./Headers/*) $(wildcard ./Headers/libc/*) $(wildcard ./Headers/klib/*))
EFI_INCLUDE := $(subst ./,-I./,$(wildcard ./Boot/Headers/*))
endif
#=Files
ifeq (${TARGET},x86_64)
C_FILES := $(subst .c,_kernel.o,$(wildcard ./Kernel/x86_64/**/*.c) $(wildcard ./Kernel/x86_64/*.c))
EFI_FILES := $(subst .c,_efi.o,$(wildcard ./Boot/x86_64/*.c))
else ifeq (${TARGET}, aarch64)
C_FILES := $(subst .c,_kernel.o,$(wildcard ./Kernel/ARM/**/*.c))
EFI_FILES = 
endif
KLIB_FILES := $(subst .c,_klib.o,$(wildcard ./klib/**/*.c))
LIBC_FILES := $(subst .c,_libc.o,$(wildcard ./libc/**/*.c))
ifeq (${TARGET},x86_64)
XASM_FILES := $(subst .asm,_asm.o,$(wildcard ./Kernel/x86_64/ASM/*.asm))
else ifeq (${TARGET}, aarch64)
ASM_FILES := $(subst .s,_arm.o,$(wildcard ./Boot/ARM/*.s))
endif
#=Rules
%_arm.o: $(notdir %.s)
	${AS} ${AS_FLAGS} ./$< -o "${OUTPUT}/Boot/$(@F)"
%_asm.o: $(notdir %.asm)
	${AS} ${XS_FLAGS} ./$< -o "${OUTPUT}/ASM/$(@F)"
%_kernel.o: $(notdir %.c)
	${CC} ${XFLAGS} ${INCLUDE} ./$< -o "${OUTPUT}/Compiled/$(@F)"
%_libc.o: $(notdir %.c)
	${CC} ${XFLAGS} ${INCLUDE} ./$< -o "${OUTPUT}/libc/x86_64/$(@F)"
%_klib.o: $(notdir %.c)
	${CC} ${XFLAGS} ${INCLUDE} ./$< -o ${OUTPUT}/klib/x86_64/$(@F)
%_efi.o: $(notdir %.c)
	${CC} ${EFI_FLAGS} ${EFI_INCLUDE} ./$< -o ${OUTPUT}/Boot/$(@F)
#=Linker
ifeq (${TARGET}, aarch64)
LINKER = arm-none-eabi-ld
else
LINKER = ld
endif
#=Other
CLN = ''

all: efi compile iso hdd hdw clean
	${EMULATOR} ${QARGS}

efi: ens check dir klib libc $(EFI_FILES)
    ifneq (${TARGET}, aarch64)
	cp ${EFI_PATH}/lib/data.o ${OUTPUT}/Boot/data.o
	${LINKER} ${EFI_LD_FLAGS} ${OUTPUT}/Boot/*.o -o ${OUTPUT}/Boot/boot.so -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 ${OUTPUT}/Boot/boot.so ${OUTPUT}/RoverOS.efi
	objcopy --only-keep-debug ${OUTPUT}/Boot/boot.so ${OTHER_PATH}/Bootloader.sym
	rm ${OUTPUT}/Boot/boot.so
    else
	${LINKER} ${ALD_FLAGS} ${OUTPUT}/Boot/*.o -o ${OUTPUT}/Compiled/armboot.elf
    endif

hdw:
    ifeq (${USE_HDD}, 1)
	mkdir -p ./HDD/Binaries
	cp ${OUTPUT}/RoverOS.efi ./HDD/Binaries/RoverOS.efi
	cp ${OUTPUT}/RoverOS.efi ./HDD/RoverOS.efi
	cp ${OUTPUT}/RoverOS.bin ./HDD/Binaries/RoverOS.bin
	cp ${OUTPUT}/RoverOS.bin ./HDD/RoverOS.bin
	cp ./Ramdisk/initrd.disk ./HDD/initrd.disk
	@$(if $(wildcard ${HDD_PATH}),cp -r ./HDD/* '${HDD_PATH}';echo 'Copied files',echo 'Please check your HDD_PATH or check that the partition is mounted')
    endif

iso:
	mkdir -p ./ISO/boot/grub
	cp ${OTHER_PATH}/grub.cfg ./ISO/boot/grub
	grub-mkrescue -o ./RoverOS.iso ./ISO
	rm -r ./ISO

#Kernel
mov:
	$(shell find ./Kernel/x86_64 -name *.o -exec mv {} ${OUTPUT} \;)
	$(shell find ./Kernel/ARM -name *.o -exec mv {} ${OUTPUT} \;)

compile: $(C_FILES) $(XASM_FILES) mov ramdisk
	$(shell find ${OUTPUT}/ASM -name *.o -exec cp {} ${OUTPUT}/Compiled \;)
	${LD} ${XLD_FLAGS} ${OUTPUT}/Compiled/*.o -o ${OUTPUT}/RoverOS.bin
	${LD} ${XLD_FLAGS} ${OUTPUT}/Compiled/*.o -o ${OUTPUT}/RoverOS.o
	objcopy --only-keep-debug ${OUTPUT}/RoverOS.o ${OTHER_PATH}/RoverOS.sym

klib: $(KLIB_FILES)
	$(shell find ${OUTPUT}/klib -name *.o -exec cp {} ${OUTPUT}/Compiled \;)

libc: $(LIBC_FILES)
	$(shell find ${OUTPUT}/libc -name *.o -exec cp {} ${OUTPUT}/Compiled \;)

ramdisk:
	$(shell rm ./Ramdisk/*.disk)
	${CC} ./Ramdisk/ramdisk.c -o ./Ramdisk/ramdisk.o ${INCLUDE}
	./Ramdisk/ramdisk.o

#Internal tools

ens:
	@$(if $(wildcard ${EFI_PATH}),,echo '[!]Please run 'make required'';echo '[!]This next error is intentional';invalidCommandExit -ok)
	@echo "Passed test"

check:
	@$(if $(shell which ${AS}),echo '${AS} is installed',echo '[!]Please install ${AS}')
	@$(if $(shell which ${LINKER}),echo '${LINKER} is installed',echo '[!]Please install ${LINKER}')
	@$(if $(shell which ${CC}),echo '${CC} is installed',echo '[!]Please install GCC or an x64 Cross Compiler')
	@$(if $(shell which ${EMULATOR}),echo '${EMULATOR} is installed', echo '[!]Please install ${EMULATOR}')
    ifeq (${TARGET},x86_64)
	@$(if $(wildcard ${OVMF_PATH}/OVMF.fd),echo 'Found ${OVMF_PATH}/OVMF.fd',$(if $(wildcard /usr/share/qemu/OVMF.fd),echo '[!]Please set OVMF_PATH to /usr/share/qemu',echo "[!]Found no OVMF file, please set the path manually"))
    endif

dir:
	$(if $(wildcard ${EFI_PATH}),,$(shell mkdir ${EFI_PATH}))
	$(if $(wildcard ${OUTPUT}),,$(shell mkdir ${OUTPUT}))
	$(if $(wildcard ${OUTPUT}/Boot),,$(shell mkdir ${OUTPUT}/Boot))
	$(if $(wildcard ${OUTPUT}/Compiled),,$(shell mkdir ${OUTPUT}/Compiled))
	$(if $(wildcard ${OUTPUT}/libc),,$(shell mkdir ${OUTPUT}/libc))
	$(if $(wildcard ${OUTPUT}/libc/x86_64),,$(shell mkdir ${OUTPUT}/libc/x86_64))
	$(if $(wildcard ${OUTPUT}/klib),,$(shell mkdir ${OUTPUT}/klib))
	$(if $(wildcard ${OUTPUT}/klib/x86_64),,$(shell mkdir ${OUTPUT}/klib/x86_64))
	$(if $(wildcard ${OUTPUT}/ASM),,$(shell mkdir ${OUTPUT}/ASM))

required: check dir
    ifneq ("$(wildcard ${GNU_EFI_PATH})","")
	$(if $(wildcard ${EFI_PATH}/Include),,mkdir ${EFI_PATH}/Include)
	cp ${GNU_EFI_PATH}/inc/*.h ${EFI_PATH}/Include
	cp -r ${GNU_EFI_PATH}/inc/x86_64 ${EFI_PATH}/Include
	cp -r ${GNU_EFI_PATH}/x86_64/lib ${EFI_PATH}
	cp -r ${GNU_EFI_PATH}/x86_64/gnuefi ${EFI_PATH}
	cp ${GNU_EFI_PATH}/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/gnuefi/elf_x86_64_efi.lds
    else
	@echo "[!]Please set GNU_EFI_PATH to a valid absolute path"
    endif
	@echo "[!]Please fix any errors, that may exist, mentioned above marked with '[!]'"

hdd:
    ifneq (${TARGET}, aarch64)
	dd if=/dev/zero of=hdd.img bs=1M count=50
	mkfs.vfat -F 32 hdd.img
	mmd -i hdd.img ::/Binaries
	mmd -i hdd.img ::/EFI
	mcopy -i hdd.img ${OUTPUT}/RoverOS.bin ::/
	mcopy -i hdd.img ./Ramdisk/initrd.disk ::/
	mcopy -i hdd.img ${OUTPUT}/RoverOS.efi ::/EFI/
    endif

clean:
	$(shell rm -r ${OUTPUT}/*.o)
	$(shell rm -r ${OUTPUT}/Compiled/*.o)
	$(shell rm -r ${OUTPUT}/klib/**/*.o)
	$(shell rm -r ${OUTPUT}/libc/**/*.o)
	$(shell rm -r ${OUTPUT}/Boot/*.o)
	$(shell rm -r ${OUTPUT}/Boot/*.so)
	$(shell rm -r ./Ramdisk/*.tmp)
	$(shell rm -r ./Ramdisk/*.o)

reset: mov clean
	$(shell rm ./Ramdisk/*.disk)
	$(if $(wildcard ${EFI_PATH}),rm -r ${EFI_PATH},)
	$(if $(wildcard ${OUTPUT}),rm -r ${OUTPUT},)
	$(if $(wildcard ${OTHER_PATH}/*.sym),rm $(wildcard ${OTHER_PATH}/*.sym),)
	$(if $(wildcard ${OTHER_PATH}/*.log),rm $(wildcard ${OTHER_PATH}/*.log),)
	$(if $(wildcard ./*.img),rm $(wildcard ./*.img),)
	$(if $(wildcard ./ISO),rm -r ./ISO,)
	$(if $(wildcard ./HDD),rm -r ./HDD,)
	$(if $(wildcard ./*.iso), rm $(wildcard ./*.iso),)

.PHONY: check required dir reset mov hdw ens klib
