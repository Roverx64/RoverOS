ifeq (${target},X64)
$(info [!]Targeting driver for AMD64)
endif

CFLAGS =  -Wall -Wextra -mgeneral-regs-only -fno-pic -mcmodel=large -fno-stack-protector -fno-stack-check -mno-red-zone -m64 -ggdb -ffreestanding -c
INCLUDE := $(subst ./../../,-I./../../, $(wildcard ./../../Headers/*) $(wildcard ./../../Headers/**/**/) $(wildcard ./../../Headers/libc/*) $(wildcard ./../../Headers/klib/*)) -I./../Headers
OUTPUT = ./Out
DFILES := $(subst .c,.o,$(wildcard ./*.c))

$(if $(wildcard ${OUTPUT}),,$(shell mkdir ${OUTPUT}))

%.o: $(notdir %.c)
	${CC} ${CFLAGS} ${INCLUDE} ./$< -o ${OUTPUT}/$@

driver: $(DFILES)
	ld -T${LDF} ${OUTPUT}/*.o -o ./testmod.mod