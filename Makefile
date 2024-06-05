-include config.mk
#====Target====#
ifeq (${arch},aarch64)
$(info [!]Targeting aarch64)
$(info [W]aarch64 is not yet supported)
TARGET = A64
EMU = ${AARCH64_EMU}
else ifeq (${arch},x86_64)
$(info [!]Targeting AMD64)
TARGET = X64
EMU = ${x86_64_EMU}
else
$(info [!]No arch specified, defaulting to x86_64)
TARGET = X64
EMU = ${x86_64_EMU}
endif
#=====Misc=====#
ifeq (${lds_patch},0)
$(info [!]Using gnuefi elf_x86_64_efi.lds)
LDS_PATCH = 0
else
LDS_PATCH = 1
endif
#==C Compiler==#
CC = ${C_COMPILER}
CR = ${CROSS_COMPILER}
ifeq (${no_cross},1)
$(info [!]Using ${CC} instead of ${CR})
CR = ${C_COMPILER}
endif
LD = ld
#=ASM Compiler=#
AS = nasm
#=EFI Compiler=#
ECC = gcc
#===AS Flags===#
AS_FLAGS = -g -Fdwarf -felf64
#====C Flags===#
CFLAGS = -Wall -Wextra -mgeneral-regs-only -fno-stack-protector -fno-stack-check -ffreestanding -m64 -mno-red-zone -c -ggdb -Wno-incompatible-pointer-types -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -m elf_x86_64 -nostdlib -T ${OTHER_PATH}/linker.ld -L./klib -L./libc
ifeq (${werror},1)
CFLAGS += -Werror
endif
#==EFI Flags===#
EFLAGS = -Wno-implicit-int -mgeneral-regs-only -Wall -Wextra -fpic -ffreestanding -fno-stack-protector -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -Wno-packed-bitfield-compat -c
ELD_FLAGS = -shared -Bsymbolic -L./gnuefi/x86_64/lib -L./gnuefi/x86_64/gnuefi ./gnuefi/x86_64/gnuefi/crt0-efi-x86_64.o
ifeq (${LDS_PATCH},1)
ELD_FLAGS += -T${OTHER_PATH}/gnuefi.lds
else
ELD_FLAGS += -T./gnuefi/gnuefi/elf_x86_64_efi.lds
endif
#====Include===#
INCLUDE := $(subst ./,-I./,$(wildcard ./Headers/*) $(wildcard ./Headers/**/**/) $(wildcard ./Headers/libc/*) $(wildcard ./Headers/klib/*)) -I./Drivers/Headers
EINCLUDE = -I${EFI_PATH}/Include -I./Bootloader/Headers -I./Headers/Media -I./Bootloader/Headers/${TARGET} -I./Headers/System -I./Headers/Memory -I./Drivers/Headers -I./Headers/Misc
RD_INCLUDE := $(subst ./,-I./,$(wildcard ./Headers/*))
#=====Qemu=====#
QARGS = -m 6G -d cpu_reset,int
QARGS += -usb -device qemu-xhci,id=xhci -device ahci,id=ahci
QARGS += -drive if=pflash,format=raw,unit=0,readonly=on,file=${OVMF_PATH}/OVMF.fd
QARGS += -D ${OTHER_PATH}/qemu.log -cdrom ./RoverOS.iso
QARGS += -serial file:${OTHER_PATH}/RoverOS.log
QARGS += -drive format=raw,file=hdd.img
QARGS += -monitor stdio ${EXPERIMENTAL_ARGS}
#====Other=====#
#GDB
ifeq (${gdb},1)
$(info [!]Enabling GDB flags)
QARGS += -s -S
endif
#KVM
ifeq (${kvm},1)
$(info [!]Using KVM)
QARGS += -enable-kvm
endif
#HDD
ifeq (${hdd},1)
USE_HDD = 1
endif
#=====Files====#
#Module makefiles
MK_FILES := $(wildcard ./Drivers/**/Makefile)
#C files
KERNEL_FILES := $(subst .c,_kernel.o,$(wildcard ./Kernel/**/*.c) $(wildcard ./Kernel/*.c) $(wildcard ./Kernel/System/ACPI/*.c) $(wildcard ./Kernel/**/${TARGET}/*.c) $(wildcard ./Kernel/**/${TARGET}/**/*.c))
LIBC_FILES := $(subst .c,_libc.o,$(wildcard ./libc/**/*.c))
#Assembly
AS_FILES := $(subst .asm,_asm.o,$(wildcard ./Kernel/ASM/X64/*.asm))
EAS_FILES := $(subst .asm,_easm.o,$(wildcard ./Bootloader/X64/*.asm))
#Bootloader
BOOT_FILES := $(subst .c,_efi.o,$(wildcard ./Bootloader/X64/*.c))
#=====Rules====#
%_arm.o: $(notdir %.s)
	${AS} ${AS_FLAGS} ./$< -o "${OUTPUT_PATH}/Bootloader/$(@F)"
%_asm.o: $(notdir %.asm)
	${AS} ${AS_FLAGS} ./$< -o "${OUTPUT_PATH}/Kernel/$(@F)"
%_easm.o: $(notdir %.asm)
	${AS} ${AS_FLAGS} ./$< -o "${OUTPUT_PATH}/Bootloader/$(@F)"
%_kernel.o: $(notdir %.c)
	${CR} ${CFLAGS} -fno-pic -mcmodel=kernel ${INCLUDE} ./$< -o "${OUTPUT_PATH}/Kernel/$(@F)"
%_libc.o: $(notdir %.c)
	${CR} ${CFLAGS} -fPIC ${INCLUDE} ./$< -o "${OUTPUT_PATH}/libc/$(@F)"
%_efi.o: $(notdir %.c)
	${CR} ${EFLAGS} ${EINCLUDE} ./$< -o ${OUTPUT_PATH}/Bootloader/$(@F)
%_mod.o: $(notdir %.c)
	${CR} ${MFLAGS} ${MINCLUDE} ./$< -o ./$(@F)

all: check polarboot bootargs fortuna ramdisk hdd iso usbcpy clean
    ifeq (${gdb},1)
	objdump -h ${OUTPUT_PATH}/Bootloader/Polarboot.efi
    endif
	${EMU} ${QARGS}

#===Builds the cpu core bootstrap
corebootstrap:
	${AS} -fbin ./Bootloader/Corebootstrap.asm -o ./Ramdisk/FS/Corebootstrap.bin

#===Builds the ISO
iso:
    ifeq (${TARGET},X64)
	mkdir -p ./ISO/boot/grub
	cp ${OTHER_PATH}/grub.cfg ./ISO/boot/grub
	grub-mkrescue -o ./RoverOS.iso ./ISO
	rm -r ./ISO
    endif

#===Copies files to USB for real hardware testing
usbcpy:
    ifeq (${usb},1)
	cp -f ${OUTPUT_PATH}/Kernel/Fortuna.bin ${USB_PATH}/Fortuna.bin
	cp -f ${OUTPUT_PATH}/Bootloader/Polarboot.efi ${USB_PATH}/Polarboot.efi
    endif

#===Builds modules/drivers
mod:
	$(info [!]Building drivers)
	$(foreach mk,${MK_FILES},$(MAKE) -C $(subst Makefile,,$(mk)) driver target=${TARGET} CC=${CR} LDF=./../../Other/driverlink.ld)

#==Copies file to tftp server
netboot:
	sudo cp ./Output/Bootloader/Polarboot.efi /srv/tftp/Polarboot.efi
	sudo cp ./Output/Kernel/Fortuna.bin /srv/tftp/Fortuna.bin

#===Builds the HDD
hdd:
    ifeq (${TARGET},X64)
	dd if=/dev/zero of=hdd.img bs=1M count=50
	mkfs.vfat -F 32 hdd.img
	mmd -i hdd.img ::/Binaries
	mmd -i hdd.img ::/EFI
	mmd -i hdd.img ::/MOD
	mcopy -i hdd.img ./Ramdisk/initrd.disk ::/
	mcopy -i hdd.img ${OUTPUT_PATH}/Bootloader/Polarboot.efi ::/EFI/
	mcopy -i hdd.img ${OUTPUT_PATH}/Kernel/Fortuna.bin ::/
	mcopy -i hdd.img ./Other/console.psf ::/
	mcopy -i hdd.img ./Other/polarboot.args ::/
	$(foreach mod,$(wildcard ./Drivers/**/*.rkx),mcopy -i hdd.img $(mod) ::/$(shell basename $(mod)))
    endif

#==Compiles bootargs
bootargs:
	${CC} ./Bootloader/processargs.c -o ./Bootloader/processargs.o -I./Bootloader/Headers/X64
	./Bootloader/processargs.o
	rm ./Bootloader/processargs.o

#===Builds the UEFI bootloader
polarboot: $(BOOT_FILES) $(EAS_FILES)
	@echo "[!]Building polarboot"
	${LD} ${ELD_FLAGS} ${OUTPUT_PATH}/Bootloader/*.o -o ${OUTPUT_PATH}/Bootloader/Polarboot.so -lgnuefi -lefi
	objcopy --only-keep-debug ${OUTPUT_PATH}/Bootloader/Polarboot.so ${OTHER_PATH}/Polarboot.sym
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc --target=efi-app-x86_64 ${OUTPUT_PATH}/Bootloader/Polarboot.so ${OUTPUT_PATH}/Bootloader/Polarboot.efi

#===Builds the kernel
fortuna: $(AS_FILES) $(KERNEL_FILES) $(LIBC_FILES)
	${LD} -T${OTHER_PATH}/kernel.ld ${OUTPUT_PATH}/Kernel/*.o -o ${OUTPUT_PATH}/Kernel/Fortuna.bin
	${LD} -T${OTHER_PATH}/kernel.ld ${OUTPUT_PATH}/Kernel/*.o -o ${OUTPUT_PATH}/Kernel/Fortuna.o
	objcopy --only-keep-debug ${OUTPUT_PATH}/Kernel/Fortuna.o ${OTHER_PATH}/Fortuna.sym

#===Builds libc
libc: $(LIBC_FILES)
	${LD} -T${OTHER_PATH}/libc.ld ${OUTPUT_PATH}/libc/*.o -o ${OUTPUT_PATH}/libc/libc.bin
	${LD} -T${OTHER_PATH}/libc.ld ${OUTPUT_PATH}/libc/*.o -o ${OUTPUT_PATH}/libc/libc.o
	cp ${OUTPUT_PATH}/libc/libc.o ${OUTPUT_PATH}/Kernel/libc.o
	objcopy --only-keep-debug ${OUTPUT_PATH}/libc/libc.o ${OTHER_PATH}/libc.sym

#===Builds the ramdisk
ramdisk:
	$(shell rm ./Ramdisk/*.disk)
	${CC} ./Ramdisk/ramdisk.c -o ./Ramdisk/ramdisk.o ${RD_INCLUDE}
	./Ramdisk/ramdisk.o

#===Check for required tools and directories
check:
	@echo [!]Checking for required tools
	@$(if $(shell command -v ${CC}),echo '[!]Found ${CC}', echo '[X]No ${CC}';failHere)
	@$(if $(shell command -v ${AS}),echo [!]Found ${AS}, echo [X]No ${AS};failhere)
	@$(if $(shell command -v ${LD}),echo [!]Found ${LD}, echo [X]No ${LD};failHere)
	@$(if $(shell command -v ${EMU}),echo [!]Found ${EMU}, echo [X]No ${EMU};failHere)
	@$(if $(shell command -v git),echo [!]Found git, echo [X]No git;failHere)
	@$(if $(shell command -v mtools),echo [!]Found mtools, echo [X]No mtools;failHere)
	@$(if $(shell command -v xorriso),echo [!]Found xorriso, echo [X]No xorriso;failHere)
	@$(if $(shell command -v mkfs.vfat),echo [!]Found mkfs.vfat, echo [X]No mkfs.vfat;failHere)
	@$(if $(shell command -v mmd),echo [!]Found mmd, echo [X]No mmd;failHere)
	@$(if $(shell command -v mcopy),echo [!]Found mcopy, echo [X]No git;failHere)
	@$(if $(shell command -v basename),echo [!]Found basename, echo [X]No basename;failHere)
	@echo [!]Checking paths
	@$(if $(wildcard ${EFI_PATH}),,$(shell mkdir ${EFI_PATH}))
	@$(if $(wildcard ${EFI_PATH}/Include),,mkdir ${EFI_PATH}/Include)
	@$(if $(wildcard ${OVMF_PATH}/OVMF.fd),echo [!]Found OVMF.fd, echo [X]No OVMF.fd;failHere)
	@$(if $(wildcard ${OUTPUT_PATH}),,$(shell mkdir ${OUTPUT_PATH}))
	@$(if $(wildcard ${OUTPUT_PATH}/Bootloader),,$(shell mkdir ${OUTPUT_PATH}/Bootloader))
	@$(if $(wildcard ${OUTPUT_PATH}/Kernel),,$(shell mkdir ${OUTPUT_PATH}/Kernel))
	@$(if $(wildcard ${OUTPUT_PATH}/libc),,$(shell mkdir ${OUTPUT_PATH}/libc))
	@echo [!]Checking for gnuefi
    ifeq ("$(wildcard ./gnuefi)","")
	@echo [!]Cloning gnuefi
	git clone ${GNUEFI_REPO} gnuefi
	@echo [!]Building gnuefi
	cd ./gnuefi && make
	cp ./gnuefi/inc/*.h ${EFI_PATH}/Include
	cp -r ./gnuefi/inc/x86_64 ${EFI_PATH}/Include
	cp -r ./gnuefi/x86_64/lib ${EFI_PATH}
	cp -r ./gnuefi/x86_64/gnuefi ${EFI_PATH}
	cp ./gnuefi/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/gnuefi/elf_x86_64_efi.lds
    else
	@echo [!]gnuefi found
    endif
#===Cleans all object files
clean:
	$(shell find . -path ./gnuefi -prune -o -name '*.o' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.mod' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.so' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.rkx' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.a' -exec rm {} \;)
	$(shell find . -name *.tmp -exec rm {} \;)
	@echo "[!]Cleaned all object files"
#===Resets directory back to its intial state
reset:
	$(if $(wildcard ${EFI_PATH}),rm -r ${EFI_PATH},)
	$(if $(wildcard ./gnuefi),rm -rf ./gnuefi,)
	$(if $(wildcard ${OUTPUT_PATH}),rm -r ${OUTPUT_PATH},)
	$(if $(wildcard ${OTHER_PATH}/*.sym),rm ${OTHER_PATH}/*.sym,)
	$(if $(wildcard ${OTHER_PATH}/*.log),rm ${OTHER_PATH}/*.log,)
	$(if $(wildcard ./*.iso),rm ./*.iso,)
	$(if $(wildcard ./Ramdisk/*.disk),rm ./Ramdisk/*.disk,)
	$(if $(wildcard ./*.img),rm ./*.img,)
	$(if $(wildcard ./ISO),rm -r ./ISO,)
	$(if $(wildcard ./HDD),rm -r ./HDD,)
	$(shell find ./Drivers/**/ -name 'Out' -exec rm -rf {} \;)
	@echo "[!]Reset folder back to the default state"

.PHONY: check hdd reset
