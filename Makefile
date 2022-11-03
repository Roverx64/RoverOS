#=====Configurable=====#
#=Options
#If the .efi file should be moved to the ESP (Sudo required)
USE_ESP = 0
#If the command uefi-run should be used
USE_UEFI_RUN = 1
#If the .bin file should be moved to
USE_HDD = 0
#=Paths
#For easy copying of required resources
GNU_EFI_PATH = NONE
#Must be FAT32 for UEFI
HDD_PATH = /media/rover/RoverOS
#For required linker/data files from gnu-efi
EFI_PATH = ./Boot/Resources
#For files like 'linker.ld' && grub.cfg
OTHER_PATH = ./Resources
#Path to ESP if USE_ESP=1
ESP_PATH = /boot/efi/EFI/RoverOS/Boot
#Path to Qemu UEFI firmware
OVMF_PATH = /usr/share/qemu/OVMF.fd
#=GCC
CC = "$$HOME/opt/cross/bin/x86_64-elf-gcc" #Path to cross compiler
CFLAGS = -Wno-format -Wno-unused-variable -Wno-incompatible-pointer-types -Wno-unused-parameter -Wno-implicit-function-declaration -ffreestanding -mcmodel=large -m64 -fno-pic -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -c -ggdb
EFI_CC = ${CC} #Compiler to use for the EFI C code
OUTPUT = ./Output
#=Qemu (Currently not working, please use uefi-run)
QARGS = -bios ${OVMF_PATH}
QARGS += -m 2G 

#===Non=Configurable===#
#=C
INCLUDE = -I./Headers \
		  -I./Headers/System \
		  -I./Headers/FS
CS_0 := $(shell find . -path ./Boot -prune -o -path ./libc -prune -o -name *.c)
CS_1 := $(subst ./Boot, ,$(CS_0))
CS_2 := $(subst ./libc, ,$(CS_1))
CF := $(subst .c,.o,$(CS_2))
#=EFI
EFI_CFLAGS = -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c
EFI_INCLUDE = -I/usr/include/efi $(INCLUDE)

%.o: $(notdir %.c)
	${CC} ${CFLAGS} ${INCLUDE} ./$< -o $@

#=LINKER
LINKER = ld
EFI_LINKER = ${LINKER}
LD_FLAGS = -m elf_x86_64 -nostdlib -T ${OTHER_PATH}/linker.ld
EFI_LD_FLAGS = -shared -Bsymbolic -L${EFI_PATH}/lib -L${EFI_PATH}/gnuefi -T${EFI_PATH}/elf_x86_64_efi.lds ${EFI_PATH}/crt0-efi-x86_64.o

all: efi esp $(CF) compile clean
    ifeq (${USE_UEFI_RUN}, 1)
	uefi-run ${OUTPUT}/Boot/RoverOS.efi -- -m 2G
    endif
	@echo "Done"

#EFI

efi:
	${EFI_CC} ${EFI_CFLAGS} ./Boot/boot.c -o ${OUTPUT}/Boot/boot.o ${EFI_INCLUDE}
	${EFI_LINKER} ${EFI_LD_FLAGS} ${OUTPUT}/Boot/*.o -o ${OUTPUT}/Boot/boot.so -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 ${OUTPUT}/Boot/boot.so ${OUTPUT}/Boot/RoverOS.efi

esp:
    ifeq (${USE_ESP}, 1)
	if sudo test -d '${ESP_PATH}'; then sudo cp ${OUTPUT}/Boot/RoverOS.efi ${ESP_PATH}/RoverOS.efi; else echo "Please update your ESP path or create 'RoverOS/Boot' in your EFI directory"; fi
    endif

required:
	cp ${GNU_EFI_PATH}/x86_64/gnuefi/crt0-efi-x86_64.o ${EFI_PATH}/crt0-efi-x86_64.o
	cp ${GNU_EFI_PATH}/x86_64/lib/data.o ${EFI_PATH}/data.o
	cp ${GNU_EFI_PATH}/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/elf_x86_64_efi.lds
	cp -r ${GNU_EFI_PATH}/x86_64/lib ${EFI_PATH}
	cp -r ${GNU_EFI_PATH}/x86_64/gnuefi ${EFI_PATH}

#C

compile:
	$(shell find ./Kernel/ -name *.o -exec mv {} ${OUTPUT} \;)
	${LD} ${LD_FLAGS} ${OUTPUT}/*.o -o ${OUTPUT}/RoverOS.bin
	${LD} ${LD_FLAGS} ${OUTPUT}/*.o -o ${OUTPUT}/RoverOS.o
	objcopy --only-keep-debug ${OUTPUT}/RoverOS.o ${OTHER_PATH}/RoverOS.sym

#Misc

clean:
	rm -r -f ${OUTPUT}/*.o


hdd:
	dd if=/dev/zero of=hdd.img bs=1M count=200
	mformat -F -i hdd.img ::

.PHONY: required clean