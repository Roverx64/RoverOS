#=====Configurable=====#
#=Options
USE_ESP = 0
USE_UEFI_RUN = 1
USE_HDD = 0
#=Paths
GNU_EFI_PATH = NONE
HDD_PATH = NONE
EFI_PATH = ./Boot/Resources
OTHER_PATH = ./Resources
ESP_PATH = NONE
OVMF_PATH = /usr/share/qemu/OVMF.fd
OUTPUT = ./Output

#===Non=Configurable===#
#=GCC
ifeq ("$(wildcard $("$$HOME/opt/cross/bin/x86_64-elf-gcc"))","")
	CC = "$$HOME/opt/cross/bin/x86_64-elf-gcc"
else
	CC = gcc
endif
EFI_CC = ${CC}
#=Flags
CFLAGS = -Wno-format -Wno-unused-variable -Wno-incompatible-pointer-types -Wno-unused-parameter -Wno-implicit-function-declaration -ffreestanding -mcmodel=large -m64 -fno-pic -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -c -ggdb
EFI_CFLAGS = -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c
LD_FLAGS = -m elf_x86_64 -nostdlib -T ${OTHER_PATH}/linker.ld
EFI_LD_FLAGS = -shared -Bsymbolic -L${EFI_PATH}/lib -L${EFI_PATH}/gnuefi -T${EFI_PATH}/gnuefi/elf_x86_64_efi.lds ${EFI_PATH}/gnuefi/crt0-efi-x86_64.o
#=Include
INCLUDE = -I./Headers \
		  -I./Headers/System \
		  -I./Headers/FS
EFI_INCLUDE = -I${EFI_PATH}/Include $(INCLUDE)
#=Files
CS_0 := $(shell find . -path ./Boot -prune -o -path ./libc -prune -o -name *.c)
CS_1 := $(subst ./Boot, ,$(CS_0))
CS_2 := $(subst ./libc, ,$(CS_1))
CF := $(subst .c,.o,$(CS_2))
#=Rules
%.o: $(notdir %.c)
	${CC} ${CFLAGS} ${INCLUDE} ./$< -o $@
#=Linker
LINKER = ld
EFI_LINKER = ${LINKER}

all: efi esp $(CF) mov compile clean
    ifeq (${USE_UEFI_RUN}, 1)
		uefi-run ${OUTPUT}/Boot/RoverOS.efi -- -m 2G
    endif

#EFI

efi:
	cp ${EFI_PATH}/lib/data.o ${OUTPUT}/Boot/data.o
	${EFI_CC} ${EFI_CFLAGS} ./Boot/boot.c -o ${OUTPUT}/Boot/boot.o ${EFI_INCLUDE}
	${EFI_LINKER} ${EFI_LD_FLAGS} ${OUTPUT}/Boot/*.o -o ${OUTPUT}/Boot/boot.so -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 ${OUTPUT}/Boot/boot.so ${OUTPUT}/Boot/RoverOS.efi

esp:
    ifeq (${USE_ESP}, 1)
	if sudo test -d '${ESP_PATH}'; then sudo cp ${OUTPUT}/Boot/RoverOS.efi ${ESP_PATH}/RoverOS.efi; else echo "Please update your ESP path or create 'RoverOS/Boot' in your EFI directory"; fi
    endif

#Kernel

mov:
	$(shell find ./Kernel/ -name *.o -exec mv {} ${OUTPUT} \;)

compile:
	${LD} ${LD_FLAGS} ${OUTPUT}/*.o -o ${OUTPUT}/RoverOS.bin
	${LD} ${LD_FLAGS} ${OUTPUT}/*.o -o ${OUTPUT}/RoverOS.o
	objcopy --only-keep-debug ${OUTPUT}/RoverOS.o ${OTHER_PATH}/RoverOS.sym

#Tools

required:
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

reset: mov clean
	if test -d ${EFI_PATH}/gnuefi; then rm -r ${EFI_PATH}/gnuefi; fi
	if test -d ${EFI_PATH}/lib; then rm -r ${EFI_PATH}/lib; fi
	if test -d ${EFI_PATH}/Include; then rm -r ${EFI_PATH}/Include; fi
	if test ${OUTPUT}/Boot/RoverOS.efi; then rm ${OUTPUT}/Boot/RoverOS.efi; fi
	if test ${OUTPUT}/RoverOS.bin; then rm ${OUTPUT}/RoverOS.bin; fi
	if test ${OTHER_PATH}/RoverOS.sym; then rm ${OTHER_PATH}/RoverOS.sym; fi


.PHONY: required clean reset mov