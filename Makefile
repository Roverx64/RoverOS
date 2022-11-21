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
QARGS = -bios ${OVMF_PATH}/OVMF.fd -m 2G -d cpu_reset
QARGS += -D ${OTHER_PATH}/qemu.log -cdrom ./RoverOS.iso
QARGS += -serial file:${OTHER_PATH}/RoverOS.log -monitor stdio
QARGS += -usb -device ahci,id=ahci
QARGS += -drive format=raw,file=hdd.img,id=hddi
QARGS += -rtc base=utc -no-reboot
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
ifneq ("$(shell which x86_64-elf-gcc)","")
CC = x86_64-elf-gcc
else
CC = gcc
endif
#=ASM
AS = nasm
AS_FLAGS = -g -Fdwarf -felf64
#=Flags
CFLAGS = -Wno-format -Wno-unused-variable -Wno-incompatible-pointer-types -Wno-unused-parameter -Wno-implicit-function-declaration -ffreestanding -mcmodel=large -m64 -fno-pic -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -c -ggdb
EFI_CFLAGS = -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -Wno-packed-bitfield-compat -c
LD_FLAGS = -m elf_x86_64 -nostdlib -T ${OTHER_PATH}/linker.ld
EFI_LD_FLAGS = -shared -Bsymbolic -L${EFI_PATH}/lib -L${EFI_PATH}/gnuefi -T${EFI_PATH}/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/gnuefi/crt0-efi-x86_64.o
#=Include
INCLUDE := $(subst ./,-I./,$(wildcard ./Headers/*) $(wildcard ./Headers/libc/*))
EFI_INCLUDE = -I${EFI_PATH}/Include -I./Boot/Include $(INCLUDE)
#=Files
EFI_FILES := $(subst .c,_efi.o,$(wildcard ./Boot/*.c))
C_FILES := $(subst .c,_kernel.o,$(wildcard ./Kernel/*.c))
LIBC_FILES := $(subst .c,_libc.o,$(wildcard ./libc/**/*.c))
ASE_FILES := $(subst .asm,_asme.o,$(wildcard ./Boot/**/*.asm))
#=Rules
#ASM
%_asme.o: $(notdir %.asm)
	${AS} ${AS_FLAGS} ./$< -o ${OUTPUT}/$(subst Assembly/,,$@)
#C
%_kernel.o: $(notdir %.c)
	${CC} ${CFLAGS} ${INCLUDE} ./$< -o ${OUTPUT}/$(subst Kernel/,,$@)
%_libc.o: $(notdir %.c)
	${CC} ${CFLAGS} ${INCLUDE} ./$< -o ${OUTPUT}/$(subst libc/,,$@)
%_efi.o: $(notdir %.c)
	${CC} ${EFI_CFLAGS} ${EFI_INCLUDE} ./$< -o ${OUTPUT}/$@
#=Linker
LINKER = ld
EFI_LINKER = ${LINKER}
#=Internal use
CLN = ''

all: efi compile iso hdd hdw clean
	${EMULATOR} ${QARGS}

#EFI

efi: ens $(EFI_FILES) $(ASE_FILES)
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
	@$(if $(wildcard ${HDD_PATH}),cp -r ./HDD/* '${HDD_PATH}';echo 'Copied files',echo 'Please check your HDD_PATH or check that the partition is mounted')
    endif

iso:
	mkdir -p ./ISO/boot/grub
	cp ${OTHER_PATH}/grub.cfg ./ISO/boot/grub
	grub-mkrescue -o ./RoverOS.iso ./ISO
	rm -r ./ISO

#Kernel

mov:
	$(shell find ./Kernel/ -name *.o -exec mv {} ${OUTPUT} \;)

compile: $(C_FILES) mov
	${LD} ${LD_FLAGS} ${OUTPUT}/*.o -o ${OUTPUT}/RoverOS.bin
	${LD} ${LD_FLAGS} ${OUTPUT}/*.o -o ${OUTPUT}/RoverOS.o
	objcopy --only-keep-debug ${OUTPUT}/RoverOS.o ${OTHER_PATH}/RoverOS.sym

#Tools

#=Ensure 'make required' was run
ens:
	@$(if $(wildcard ${EFI_PATH}),,echo '[!]Please run 'make required'';echo '[!]This next error is intentional';invalidCommandExit -ok)
	@echo "Passed test"

check:
	@$(if $(shell which ${AS}),echo '${AS} is installed',echo '[!]Please install ${AS}')
	@$(if $(shell which ${CC}),echo '${CC} is installed',echo '[!]Please install GCC or an x64 Cross Compiler')
	@$(if $(shell which ${EMULATOR}),echo '${EMULATOR} is installed', echo '[!]Please install ${EMULATOR}')
	@$(if $(wildcard ${OVMF_PATH}/OVMF.fd),echo 'Found ${OVMF_PATH}/OVMF.fd',$(if $(wildcard /usr/share/qemu/OVMF.fd),echo '[!]Please set OVMF_PATH to /usr/share/qemu',echo "[!]Found no OVMF file, please set the path manually"))


dir:
	$(if $(wildcard ${EFI_PATH}),,$(shell mkdir ${EFI_PATH}))
	$(if $(wildcard ${OUTPUT}),,$(shell mkdir ${OUTPUT}))
	$(if $(wildcard ${OUTPUT}/Boot),,$(shell mkdir ${OUTPUT}/Boot))
	$(if $(wildcard ${OUTPUT}/libc),,$(shell mkdir ${OUTPUT}/libc))

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
	dd if=/dev/zero of=hdd.img bs=1M count=50
	mkfs.vfat -F 32 hdd.img
	mmd -i hdd.img ::/Binaries
	mmd -i hdd.img ::/EFI
	mcopy -i hdd.img ${OUTPUT}/RoverOS.bin ::/Binaries/
	mcopy -i hdd.img ${OUTPUT}/RoverOS.bin ::/
	mcopy -i hdd.img ${OUTPUT}/Boot/RoverOS.efi ::/EFI/

clean:
	$(shell rm -r ${OUTPUT}/*.o)
	$(shell rm -r ${OUTPUT}/Boot/*.o)
	$(shell rm -r ${OUTPUT}/Boot/*.so)

reset: mov clean
	$(if $(wildcard ${EFI_PATH}),rm -r ${EFI_PATH},)
	$(if $(wildcard ${OUTPUT}),rm -r ${OUTPUT},)
	$(if $(wildcard ${OTHER_PATH}/*.sym),rm $(wildcard ${OTHER_PATH}/*.sym),)
	$(if $(wildcard ${OTHER_PATH}/*.log),rm $(wildcard ${OTHER_PATH}/*.log),)
	$(if $(wildcard ./*.img),rm $(wildcard ./*.img),)
	$(if $(wildcard ./ISO),rm -r ./ISO,)
	$(if $(wildcard ./HDD),rm -r ./HDD,)
	$(if $(wildcard ./*.iso), rm $(wildcard ./*.iso),)

.PHONY: check required dir reset mov hdw ens