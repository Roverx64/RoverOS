ENTRY(bootstrapEntry)
SECTIONS{
    . = 0x8000;
    __loadaddr = .;
    .text : {*(.text)}
    .data : {*(.data) *(.rodata)}
    .bss : ALIGN(0x1000){*(.bss) *(COMMON)}
    /*Stack space*/
    . = ALIGN(0x1000);
    . = . + 0x1000;
    __stackTop = .;
    /*Small amount of memory for the bootloader mmap*/
    __bootmem = .;
    . = . +0x1000;
    __memsize = .-__bootmem;
    __loadersize = .;
    /DISCARD/:{*(.comment) *(.eh_frame)}
}