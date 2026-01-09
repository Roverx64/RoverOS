LFLAGS = -Wall -Wextra -mgeneral-regs-only -fpic -fno-stack-protector -fno-stack-check -ffreestanding -m64 -mno-red-zone -c -ggdb
LFILES = $(subst .c,_ruina.o,$(wildcard ${LPATH}/ruina/*.c))
LHEADERS = ${KHEADERS} ${LPATH}/ruina/Headers
LDFLAGS = -shared

%_ruina.o: $(notdir %.c)
	${CC} ${LFLAGS} ${LHEADERS} ./$< -o ./$(@F)

all: $(LFILES)
	$(info [!]Building ruina)
	ld -Bsymbolic -l${KPATH} -lfortuna

clean:

.PHONY: all clean