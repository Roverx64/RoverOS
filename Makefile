-include config.mk

#Determine arch
ifeq (${arch},aarch64)
    $(info [!]Targeting aarch64)
    $(info [!]This is only compatible with a rpi4 (BCM2711))
    $(info [W]aarch64 is in development)
    TARGET = aarch64
else
    $(info [!]Targeting x86_64)
    TARGET = x86_64
endif

#===Arch specific stuff===#
QARGS = 
#==Compiler
ifneq (${no_cross},1)
    ifeq (${TARGET},aarch64)
        CR = ${A64_COMPILER}
    else
        CR = ${X64_COMPILER}
    endif
else
    $(info [!]Using ${CC} instead of ${CR})
    CR = ${C_COMPILER}
endif
#==Other
ifeq (${TARGET},aarch64)
    EMU = ${A64_EMU}
    BC = ${A64_COMPILER}
    LD = ${A64_LD}
    BLDFLAGS = -m armelf -nostdlib -T ${OTHER_PATH}/ld/polarboot-aarch64.ld
    BINCLUDE = -I./Headers/Media -I./Bootloader/${TARGET}/Headers -I./Headers/System -I./Headers/Memory -I./Headers/Devices/arch/aarch64 -I./Headers/System/arch/aarch64 -I./Headers/libc/std
    BFLAGS = -Wno-implicit-int -mgeneral-regs-only -Wall -Wextra -ffreestanding -fno-stack-protector -Wno-packed-bitfield-compat -c
    AS = ${A64_AS}
    ASFLAGS = -EL -mcpu=cortex-a72 -g
    BAS_FILES := $(subst .s,_barm.o,$(wildcard ./Bootloader/${TARGET}/*.s))
    QARGS += -m 2G
else #x86_64
    EMU = ${X64_EMU}
    BC = ${CR}
    LD = ${X64_LD}
    BINCLUDE = -I${EFI_PATH}/Include -I./Headers/Media -I./Bootloader/${TARGET}/Headers/ -I./Headers/System -I./Headers/Memory -I./Drivers/Headers -I./Headers/Misc
    BFLAGS = -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -Wno-implicit-int -fshort-wchar -mgeneral-regs-only -Wall -Wextra -fpic -ffreestanding -fno-stack-protector -g -mno-red-zone -Wno-packed-bitfield-compat
    AS = ${X64_AS}
    ASFLAGS = -g -Fdwarf -felf64 -gdwarf
    BAS_FILES := $(subst .asm,_basm.o,$(wildcard ./Bootloader/${TARGET}/*.asm))
    QARGS += -m 6G
    QARGS += -drive if=pflash,format=raw,unit=0,readonly=on,file=${OVMF_PATH}/OVMF.fd
    QARGS += -D ${OTHER_PATH}/log/qemu.log -cdrom ./RoverOS.iso
    QARGS += -drive format=raw,file=hdd.img
endif
QARGS += -usb -device qemu-xhci,id=xhci -device ahci,id=ahci
QARGS += -serial file:${OTHER_PATH}/log/RoverOS.log
QARGS += -monitor stdio ${EXPERIMENTAL_ARGS}
DMK_FILES := $(wildcard ./Drivers/${TARGET}/**/Makefile) $(wildcard ./Drivers/both/**/Makefile)
LMK_FILES := 
KERNEL_FILES := $(subst .c,_kernel.o,$(wildcard ./Kernel/**/*.c) $(wildcard ./Kernel/Lienzo/image/*) $(wildcard ./Kernel/*.c) $(wildcard ./Kernel/System/ACPI/*.c) $(wildcard ./Kernel/**/arch/${TARGET}/*.c) $(wildcard ./Kernel/**/${TARGET}/*.c))
LIBC_FILES := $(subst .c,_libc.o,$(wildcard ./libc/**/*.c))
RD_INCLUDE := $(subst ./,-I./,$(wildcard ./Headers/*))
LDFLAGS = -m elf_${TARGET} -nostdlib -T ${OTHER_PATH}/ld/kernel-${TARGET}.ld -L./klib -L./libc
INCLUDE := $(subst ./,-I./,$(wildcard ./Headers/*) $(wildcard ./Headers/**/**/) $(wildcard ./Headers/libc/*) $(wildcard ./Headers/klib/*) $(wildcard ./Headers/**/arch/${TARGET}) $(wildcard ./Library/**) $(wildcard ./Headers/Lienzo/image)) -I./Drivers/Headers 
BFILES := $(wildcard ./Bootloader/${TARGET}/*.c)
AS_FILES := $(subst .asm,_asm.o,$(wildcard ./Kernel/ASM/${TARGET}/*.asm)) $(subst .s,_arm.o,$(wildcard ./Kernel/ASM/${TARGET}/*.s))
CFLAGS = -Wall -Wextra -mgeneral-regs-only -fno-omit-frame-pointer -fno-stack-protector -fno-stack-check -ffreestanding -m64 -mno-red-zone -c -ggdb -Wno-incompatible-pointer-types -Wno-unused-parameter -Wno-unused-variable
BFLAGS += -DBOOTLOADER_COMPILE_STAGE
ifeq (${werror},1)
CFLAGS += -Werror
endif
#==SEFI options
SFILES = $(wildcard ./Bootloader/SEFI/src/*.c)
SINCLUDE = -I./Bootloader/SEFI/Headers
#=====User options=====#
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
#=====Rules====#
%_barm.o: $(notdir %.s)
	${AS} ${ASFLAGS} ./$< -o "${OUTPUT_PATH}/Bootloader/$(@F)"
%_arm.o: $(notdir %.s)
	${AS} ${ASFLAGS} ./$< -o "${OUTPUT_PATH}/Kernel/$(@F)"
%_basm.o: $(notdir %.asm)
	${AS} ${ASFLAGS} ./$< -o "${OUTPUT_PATH}/Bootloader/$(@F)"
%_asm.o: $(notdir %.asm)
	${AS} ${ASFLAGS} ./$< -o "${OUTPUT_PATH}/Kernel/$(@F)"
%_kernel.o: $(notdir %.c)
	${CR} ${CFLAGS} -fno-pic -mcmodel=kernel ${INCLUDE} ./$< -o "${OUTPUT_PATH}/Kernel/$(@F)"
%_libc.o: $(notdir %.c)
	${CR} ${CFLAGS} -fPIC ${INCLUDE} ./$< -o "${OUTPUT_PATH}/libc/$(@F)"
%_boot.o: $(notdir %.c)
	${BC} ${BFLAGS} ${BINCLUDE} ./$< -o ${OUTPUT_PATH}/Bootloader/$(@F)
%_tboot.o: $(notdir %.c)
	${BC} ${BFLAGS} ${BINCLUDE} ./$< -o ${OUTPUT_PATH}/Bootloader/$(@F)
%_sefi.o: $(notdir %.c)
	${BC} ${BFLAGS} ${SINCLUDE} ./$< -o ${OUTPUT_PATH}/Bootloader/$(@F)
%_mod.o: $(notdir %.c)
	${CR} ${FLAGS} ${INCLUDE} ./$< -o ./$(@F)


all: check polarboot fortuna ramdisk hdd iso usbcpy
    ifeq (${TARGET},x86_64)
    ifeq (${gdb},1)
	objdump -h ${OUTPUT_PATH}/Bootloader/Polarboot.efi
    endif
    endif
	${EMU} ${QARGS}

start:
	${EMU} ${QARGS}

#===Builds the cpu core bootstrap
corebootstrap:
    ifeq (${TARGET},x86_64)
	${AS} -fbin ./Bootloader/Corebootstrap.asm -o ./Ramdisk/FS/Corebootstrap.bin
    endif

#===Builds the ISO
iso:
    ifeq (${TARGET},x86_64)
	mkdir -p ./ISO/boot/grub
	cp ${OTHER_PATH}/grub.cfg ./ISO/boot/grub
	grub-mkrescue -o ./RoverOS.iso ./ISO
	rm -r ./ISO
    endif

#===Copies files to USB for real hardware testing
usbcpy:
    ifeq (${usb},1)
	cp -f ${OUTPUT_PATH}/Kernel/Fortuna.bin ${USB_PATH}/Fortuna.bin
    ifeq (${TARGET},x86_64)
	cp -f ${OUTPUT_PATH}/Bootloader/Polarboot.efi ${USB_PATH}/Polarboot.efi
    endif
    endif

#===Builds drivers
drivers:
	$(info [!]Building drivers)
	$(foreach mk,${MK_FILES},$(MAKE) -C $(subst Makefile,,$(mk)) driver target=${TARGET} CC=${CR} LDF=./../../Other/driverlink.ld)

#===Builds the HDD
hdd:
    ifeq (${TARGET},x86_64)
	dd if=/dev/zero of=hdd.img bs=1M count=50
	mkfs.vfat -F 32 hdd.img
	mmd -i hdd.img ::/Binaries
	mmd -i hdd.img ::/EFI
	mmd -i hdd.img ::/MOD
	mcopy -i hdd.img ./Ramdisk/init.rd ::/
	mcopy -i hdd.img ./Other/sym/Fortuna.sym ::/
	mcopy -i hdd.img ${OUTPUT_PATH}/Bootloader/Polarboot.efi ::/EFI/
	mcopy -i hdd.img ${OUTPUT_PATH}/Binaries/Fortuna.bin ::/
	mcopy -i hdd.img ./Other/console.psf ::/
	$(foreach mod,$(wildcard ./Drivers/**/*.rkx),mcopy -i hdd.img $(mod) ::/$(shell basename $(mod)))
    endif

#===Builds the bootloader
polarboot:
	x86_64-w64-mingw32-gcc ${BFLAGS} ${SFILES} ${BFILES} -o ${OUTPUT_PATH}/Bootloader/Polarboot.efi ${SINCLUDE} ${BINCLUDE}
	objcopy --only-keep-debug ${OUTPUT_PATH}/Bootloader/Polarboot.efi ${OTHER_PATH}/sym/Polarboot.sym

#===Builds the kernel
fortuna: $(AS_FILES) $(KERNEL_FILES) $(LIBC_FILES)
	${LD} ${LDFLAGS} ${OUTPUT_PATH}/Kernel/*.o -o ${OUTPUT_PATH}/Binaries/Fortuna.bin
	${LD} ${LDFLAGS} ${OUTPUT_PATH}/Kernel/*.o -o ${OUTPUT_PATH}/Binaries/Fortuna.o
	${LD} ${LDFLAGS} ${OUTPUT_PATH}/Kernel/*.o -o ${OUTPUT_PATH}/Binaries/Fortuna.a
	objcopy --only-keep-debug ${OUTPUT_PATH}/Binaries/Fortuna.o ${OTHER_PATH}/sym/Fortuna.sym
	cp ${OUTPUT_PATH}/Binaries/Fortuna.bin ${RAMDISK_ROOT}/Fortuna.bin

#===Builds libc
libc: $(LIBC_FILES)
	${LD} -T${OTHER_PATH}/ld/libc.ld ${OUTPUT_PATH}/libc/*.o -o ${OUTPUT_PATH}/libc/libc.bin
	${LD} -T${OTHER_PATH}/ld/libc.ld ${OUTPUT_PATH}/libc/*.o -o ${OUTPUT_PATH}/libc/libc.o
	cp ${OUTPUT_PATH}/libc/libc.o ${OUTPUT_PATH}/Kernel/libc.o
	objcopy --only-keep-debug ${OUTPUT_PATH}/libc/libc.o ${OTHER_PATH}/sym/libc.sym

#===Builds the ramdisk
ramdisk:
	$(shell rm ./*.rd)
	${CC} ./Ramdisk/ramdisk.c -o ./Ramdisk/ramdisk.o -D_GNU_SOURCE ${RD_INCLUDE}
	./Ramdisk/ramdisk.o ${RAMDISK_ROOT}

#===Builds kernel libraries
libraries:
	$(foreach library,$(wildcard ./Library/**/*.rkx),cp $(library) ./Ramdisk/FS/Library/$(basename $(mod)))

#===Check for required tools and directories
check:
	@echo [!]Checking for required tools
	@$(if $(shell command -v ${CC}),echo '[!]Found ${CC}', echo '[X]No ${CC}';failHere)
	@$(if $(shell command -v ${AS}),echo [!]Found ${AS}, echo [X]No ${AS};failhere)
	@$(if $(shell command -v ${LD}),echo [!]Found ${LD}, echo [X]No ${LD};failHere)
	@$(if $(shell command -v ${EMU}),echo [!]Found ${EMU}, echo [X]No ${EMU};failHere)
    ifeq (${TARGET},x86_64)
	@$(if $(shell command -v ${X64_COMPILER}),echo [!]Found ${X64_COMPILER}, echo [X]No ${X64_COMPILER};failHere)
	@$(if $(shell command -v git),echo [!]Found git, echo [X]No git;failHere)
	@$(if $(shell command -v mtools),echo [!]Found mtools, echo [X]No mtools;failHere)
	@$(if $(shell command -v xorriso),echo [!]Found xorriso, echo [X]No xorriso;failHere)
    else
	@$(if $(shell command -v ${A64_COMPILER}),echo [!]Found ${A64_COMPILER}, echo [X]No ${A64_COMPILER};failHere)
    endif
	@$(if $(shell command -v mkfs.vfat),echo [!]Found mkfs.vfat, echo [X]No mkfs.vfat;failHere)
	@$(if $(shell command -v mmd),echo [!]Found mmd, echo [X]No mmd;failHere)
	@$(if $(shell command -v mcopy),echo [!]Found mcopy, echo [X]No git;failHere)
	@$(if $(shell command -v basename),echo [!]Found basename, echo [X]No basename;failHere)
	@$(if $(shell command -v x86_64-w64-mingw32-gcc),echo [!]Found x86_64-w64-mingw32-gcc, echo [X]No x86_64-w64-mingw32-gcc;failHere)
	@echo [!]Checking paths
	@$(if $(wildcard ${OUTPUT_PATH}),,$(shell mkdir ${OUTPUT_PATH}))
	@$(if $(wildcard ${OUTPUT_PATH}/Bootloader),,$(shell mkdir ${OUTPUT_PATH}/Bootloader))
	@$(if $(wildcard ${OUTPUT_PATH}/Kernel),,$(shell mkdir ${OUTPUT_PATH}/Kernel))
	@$(if $(wildcard ${OUTPUT_PATH}/libc),,$(shell mkdir ${OUTPUT_PATH}/libc))
	@$(if $(wildcard ${OUTPUT_PATH}/Binaries),,$(shell mkdir ${OUTPUT_PATH}/Binaries))
    ifeq (${TARGET},x86_64)
	@$(if $(wildcard ${OVMF_PATH}/OVMF.fd),echo [!]Found OVMF.fd, echo [X]No OVMF.fd;failHere)
	@echo [!]Checking for SEFI
    ifeq ("$(wildcard ./Bootloader/SEFI)","")
	@echo [!]Cloning SEFI
	git clone ${EFI_REPO} ./Bootloader/SEFI
    else
	@echo [!]SEFI found
    endif
    endif
#===Cleans all object files
clean:
	$(shell find . -path ./gnuefi -prune -o -name '*.o' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.mod' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.so' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.rkx' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.a' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.efi' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.iso' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.tag' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.pl' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.rd' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.bin' -exec rm {} \;)
	$(shell find . -path ./gnuefi -prune -o -name '*.log' -exec rm {} \;)
	$(shell find . -name *.tmp -exec rm {} \;)
	@echo "[!]Cleaned all object files"
	$(shell rm -r ./Documentation/*)
	$(shell rm -r ./Bootloader/SEFI/Documentation/*)
#===Resets directory back to its intial state
reset:
	$(if $(wildcard ${EFI_PATH}),rm -rf ${EFI_PATH},)
	$(if $(wildcard ${OUTPUT_PATH}),rm -r ${OUTPUT_PATH},)
	$(if $(wildcard ${OTHER_PATH}/sym/*.sym),rm ${OTHER_PATH}/sym/*.sym,)
	$(if $(wildcard ${OTHER_PATH}/log/*.log),rm ${OTHER_PATH}/log/*.log,)
	$(if $(wildcard ./*.iso),rm ./*.iso,)
	$(if $(wildcard ./*.rd),rm ./*.rd,)
	$(if $(wildcard ./*.img),rm ./*.img,)
	$(if $(wildcard ./ISO),rm -r ./ISO,)
	$(if $(wildcard ./HDD),rm -r ./HDD,)
	$(shell find ./Drivers/**/ -name 'Out' -exec rm -rf {} \;)
	$(shell rm -r ./Documentation/)
	@echo "[!]Reset folder back to the default state"

#===Creates docs
docs:
	$(if $(wildcard ./Documentation),,mkdir ./Documentation)
	$(if $(wildcard ./Bootloader/SEFI/Documentation),,mkdir ./Bootloader/SEFI/Documentation/)
	cd ./Bootloader/SEFI; doxygen
	curl https://raw.githubusercontent.com/rfoos/doxygen/refs/heads/master/asm4doxy.pl > ./asm4doxy.pl
	chmod +x ./asm4doxy.pl
	./asm4doxy.pl -ud -od $(wildcard ./Kernel/ASM/x86_64/*.asm) $(wildcard ./Kernel/ASM/aarch64/*.s) ./Bootloader/coreboot.asm
	cd ./../..
	doxygen
	find . -name '*-asm.c' -exec rm {} \;
	rm ./asm4doxy.pl
	@echo "[!]Generated docs"

.PHONY: check hdd reset ramdisk polarsefi iso docs
