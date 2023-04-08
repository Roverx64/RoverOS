-include config.mk
#======Args======#
#Arch
ifeq (${arch},aarch64)
$(info [!]Targeting aarch64)
target_arch = A64
EMULATOR = ${AARCH64_EMU}
else ifeq (${arch},x86_64)
$(info [!]Targeting x86_64)
target_arch := X64
EMULATOR = ${x86_64_EMU}
else
$(info [!]No target specified, defaulting arch to x86_64)
target_arch := X64
EMULATOR = ${x86_64_EMU}
endif
#GDB
ifneq (${gdb},1)
gdb=0
endif
#HDD
ifeq (${hdd},1)
USE_HDD=1
else
USE_HDD=0
endif
#KVM
ifeq (${kvm},1)
EMULATOR = ${KVM_EMU}
endif
#==Arch specific=#
ifeq (${target_arch},A64)
CFLAGS = None
LD_FLAGS = -T ${OTHER_PATH}/armlinker.ld
LINKER = ${A64_LD}
AS = ${A64_AS}
AS_FLAGS = -march=armv8-a -mcpu=cortex-a32
else
CFLAGS = -mno-mmx -mcmodel=large -fno-pic -mno-sse -mno-sse2 -Wno-format -Wno-unused-variable -Wno-incompatible-pointer-types -Wno-unused-parameter -Wno-implicit-function-declaration -ffreestanding -m64 -mno-red-zone -c -ggdb
EFI_FLAGS = -Wno-implicit-int -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -Wno-packed-bitfield-compat -c
EFI_LD_FLAGS = -shared -Bsymbolic -L${EFI_PATH}/lib -L${EFI_PATH}/gnuefi -T${EFI_PATH}/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/gnuefi/crt0-efi-x86_64.o
LD_FLAGS = -m elf_x86_64 -nostdlib -T ${OTHER_PATH}/linker.ld -L./klib -L./libc
LINKER = ${x64_LD}
AS = ${x64_AS}
AS_FLAGS = -g -Fdwarf -felf64
endif
#====Compilers===#
ifneq ("$(shell which x86_64-elf-gcc)","")
CC = ${C_CROSS_COMPILER}
else
CC = ${C_COMPILER}
endif
#===Qemu flags===#
ifeq (${target_arch},A64)
QARGS = -M ${ARM_MACHINE} -cpu cortex-a15 -kernel ${OUTPUT_PATH}/Compiled/armboot.elf
QARGS += -serial file:${OTHER_PATH}/RoverOS.log
else
QARGS = -bios ${OVMF_PATH}/OVMF.fd -m 500M -d cpu_reset,int
QARGS += -D ${OTHER_PATH}/qemu.log -cdrom ./RoverOS.iso
QARGS += -serial file:${OTHER_PATH}/RoverOS.log
QARGS += -usb -device ahci,id=ahci
QARGS += -drive format=raw,file=hdd.img,id=hddi
QARGS += -rtc base=utc -monitor stdio
endif
#=====Include====#
INCLUDE := $(subst ./,-I./,$(wildcard ./Headers/*) $(wildcard ./Headers/libc/*) $(wildcard ./Headers/klib/*))
EFI_INCLUDE = -I${EFI_PATH}/Include -I./Bootloader/Include -I./Bootloader/Headers/${target_arch} -I./Headers/System -I./Headers/Memory
#======Files=====#
C_FILES := $(subst .c,_kernel.o,$(wildcard ./Kernel/**/*.c) $(wildcard ./Kernel/*.c) $(wildcard ./Kernel/**/${target_arch}/*.c))
LIBC_FILES := $(subst .c,_libc.o,$(wildcard ./libc/**/*.c))
KLIB_FILES := $(subst .c,_klib.o,$(wildcard ./klib/**/*.c))
ifeq (${target_arch},X64)
BOOT_FILES := $(subst .c,_efi.o,$(wildcard ./Bootloader/X64/*.c))
ASM_FILES := $(subst .asm,_asm.o,$(wildcard ./Kernel/ASM/X64/*.asm))
else
BOOT_FILES := $(subst .s,_barm.o,$(wildcard ./Boot/A64/*.s))
ASM_FILES := $(subst .s,_arm.o,$(wildcard ./Kernel/ASM/A64/*.s))
endif
#======Rules=====#
%_arm.o: $(notdir %.s)
	${AS} ${AS_FLAGS} ./$< -o "${OUTPUT_PATH}/Bootloader/$(@F)"
%_asm.o: $(notdir %.asm)
	${AS} ${AS_FLAGS} ./$< -o "${OUTPUT_PATH}/Kernel/$(@F)"
%_kernel.o: $(notdir %.c)
	${CC} ${CFLAGS} ${INCLUDE} ./$< -o "${OUTPUT_PATH}/Kernel/$(@F)"
%_libc.o: $(notdir %.c)
	${CC} ${CFLAGS} ${INCLUDE} ./$< -o "${OUTPUT_PATH}/Kernel/$(@F)"
%_klib.o: $(notdir %.c)
	${CC} ${CFLAGS} ${INCLUDE} ./$< -o ${OUTPUT_PATH}/Kernel/$(@F)
%_efi.o: $(notdir %.c)
	${CC} ${EFI_FLAGS} ${EFI_INCLUDE} ./$< -o ${OUTPUT_PATH}/Bootloader/$(@F)
#======Funcs=====#
all: check bootloader libc klib kernel ramdisk hdd iso hdw clean
	${EMULATOR} ${QARGS}

kernel: $(C_FILES) $(ASM_FILES)
	${LD} ${LD_FLAGS} ${OUTPUT_PATH}/Kernel/*.o -o ${OUTPUT_PATH}/Compiled/Fortuna.bin
	${LD} ${LD_FLAGS} ${OUTPUT_PATH}/Kernel/*.o -o ${OUTPUT_PATH}/Compiled/Fortuna.o
	objcopy --only-keep-debug ${OUTPUT_PATH}/Compiled/Fortuna.o ${OTHER_PATH}/Fortuna.sym

bootloader: $(BOOT_FILES)
	@echo "[!]Compiling ${target_arch} bootloader"
    ifeq (${target_arch},X64)
	cp ${EFI_PATH}/lib/data.o ${OUTPUT_PATH}/Bootloader/data.o
	${LINKER} ${EFI_LD_FLAGS} ${OUTPUT_PATH}/Bootloader/*.o -o ${OUTPUT_PATH}/Bootloader/boot.so -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 ${OUTPUT_PATH}/Bootloader/boot.so ${OUTPUT_PATH}/Compiled/ROSBootloader.efi
	objcopy --only-keep-debug ${OUTPUT_PATH}/Bootloader/boot.so ${OTHER_PATH}/Bootloader.sym
	rm ${OUTPUT_PATH}/Bootloader/boot.so
    else
	${LINKER} ${LD_FLAGS} ${OUTPUT_PATH}/Bootloader/*.o -o ${OUTPUT_PATH}/Compiled/ROSBootloader.elf
    endif
	@echo "[!]Compiled ${target_arch} bootloader"

libc: $(LIBC_FILES)
	@echo "[!]Compiled libc"

klib: $(KLIB_FILES)
	@echo "[!]Compiled klib"

ramdisk:
	$(shell rm ./Ramdisk/*.disk)
	${CC} ./Ramdisk/ramdisk.c -o ./Ramdisk/ramdisk.o ${INCLUDE}
	./Ramdisk/ramdisk.o

hdw:
    ifeq (${USE_HDD},1)
	@$(if $(wildcard ${HDD_PATH}),,echo '[X]Please check your HDD_PATH or check that the partition is mounted';failHere)
	cp ${OUTPUT_PATH}/Compiled/ROSBootloader.* ${HDD_PATH}/
	cp ${OUTPUT_PATH}/Compiled/Fortuna.bin ${HDD_PATH}/
	cp ./Ramdisk/initrd.disk ${HDD_PATH}/initrd.disk
	@echo "[!]Copied files to partition"
    endif

check:
	@echo "[!]Checking for requirements"
#   #===Commands===#
	@$(if $(shell which ${AS}),echo '[!]${AS} is installed',echo '[X]Please install ${AS}';failHere)
	@$(if $(shell which ${LINKER}),echo '[!]${LINKER} is installed',echo '[X]Please install ${LINKER}';failHere)
	@$(if $(shell which ${CC}),echo '[!]${CC} is installed',echo '[X]${CC} not found, please install GCC or an x64 Cross Compiler';failHere)
	@$(if $(shell which ${EMULATOR}),echo '[!]${EMULATOR} is installed', echo '[X]Please install ${EMULATOR}';failHere)
    ifeq (${TARGET},x86_64)
	@$(if $(wildcard ${OVMF_PATH}/OVMF.fd),echo '[!]Found ${OVMF_PATH}/OVMF.fd',$(if $(wildcard /usr/share/qemu/OVMF.fd),echo '[X]Please set OVMF_PATH to /usr/share/qemu'; failHere,echo "[X]Found no OVMF file, please set the path manually"; failHere))
    endif
    ifeq ($(wildcard ${EFI_PATH}),)
#   #===Directory==#
	$(if $(wildcard ${EFI_PATH}),,$(shell mkdir ${EFI_PATH}))
	$(if $(wildcard ${OUTPUT_PATH}),,$(shell mkdir ${OUTPUT_PATH}))
	$(if $(wildcard ${OUTPUT_PATH}/Bootloader),,$(shell mkdir ${OUTPUT_PATH}/Bootloader))
	$(if $(wildcard ${OUTPUT_PATH}/Compiled),,$(shell mkdir ${OUTPUT_PATH}/Compiled))
	$(if $(wildcard ${OUTPUT_PATH}/Kernel),,$(shell mkdir ${OUTPUT_PATH}/Kernel))
#   #===GNU EFI====#
    ifneq ("$(wildcard ${GNU_EFI_PATH})","")
	@echo "[!]Copying gnuefi"
	$(if $(wildcard ${EFI_PATH}/Include),,mkdir ${EFI_PATH}/Include)
	cp ${GNU_EFI_PATH}/inc/*.h ${EFI_PATH}/Include
	cp -r ${GNU_EFI_PATH}/inc/x86_64 ${EFI_PATH}/Include
	cp -r ${GNU_EFI_PATH}/x86_64/lib ${EFI_PATH}
	cp -r ${GNU_EFI_PATH}/x86_64/gnuefi ${EFI_PATH}
	cp ${GNU_EFI_PATH}/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/gnuefi/elf_x86_64_efi.lds
    else
	@echo "[X]Please set GNU_EFI_PATH to a valid absolute path"
    endif
	@echo "[!]Please fix any errors, that may exist, mentioned above marked with '[X]'"
    endif

iso:
    ifeq (${target_arch},X64)
	mkdir -p ./ISO/boot/grub
	cp ${OTHER_PATH}/grub.cfg ./ISO/boot/grub
	grub-mkrescue -o ./RoverOS.iso ./ISO
	rm -r ./ISO
    endif

hdd:
    ifeq (${target_arch},X64)
	dd if=/dev/zero of=hdd.img bs=1M count=50
	mkfs.vfat -F 32 hdd.img
	mmd -i hdd.img ::/Binaries
	mmd -i hdd.img ::/EFI
	mcopy -i hdd.img ${OUTPUT_PATH}/Compiled/Fortuna.bin ::/
	mcopy -i hdd.img ./Ramdisk/initrd.disk ::/
	mcopy -i hdd.img ${OUTPUT_PATH}/Compiled/ROSBootloader.* ::/EFI/
    endif

clean:
	$(shell find . -name *.o -exec rm {} \;)
	$(shell find . -name *.so -exec rm {} \;)
	$(shell find . -name *.tmp -exec rm {} \;)
	@echo "[!]Cleaned all object files"
	$(if $(wildcard ${EFI_PATH}/Include),,mkdir ${EFI_PATH}/Include)
	cp ${GNU_EFI_PATH}/inc/*.h ${EFI_PATH}/Include
	cp -r ${GNU_EFI_PATH}/inc/x86_64 ${EFI_PATH}/Include
	cp -r ${GNU_EFI_PATH}/x86_64/lib ${EFI_PATH}
	cp -r ${GNU_EFI_PATH}/x86_64/gnuefi ${EFI_PATH}
	cp ${GNU_EFI_PATH}/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/gnuefi/elf_x86_64_efi.lds
	@echo "[!]Copied back gnuefi files"

reset: clean
	$(if $(wildcard ${EFI_PATH}),rm -r ${EFI_PATH},)
	$(if $(wildcard ${OUTPUT_PATH}),rm -r ${OUTPUT_PATH},)
	$(if $(wildcard ${OTHER_PATH}/*.sym),rm ${OTHER_PATH}/*.sym,)
	$(if $(wildcard ${OTHER_PATH}/*.log),rm ${OTHER_PATH}/*.log,)
	$(if $(wildcard ./*.iso),rm ./*.iso,)
	$(if $(wildcard ./Ramdisk/*.disk),rm ./Ramdisk/*.disk,)
	$(if $(wildcard ./*.img),rm ./*.img,)
	$(if $(wildcard ./ISO),rm -r ./ISO,)
	$(if $(wildcard ./HDD),rm -r ./HDD,)
	@echo "[!]Reset folder back to the default state"

.PHONY: check libc klib clean