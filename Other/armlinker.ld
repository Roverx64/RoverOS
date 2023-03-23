ENTRY(bootstrapStart)
SECTIONS{
    . = 0x80010000;
    .text   : { *(.text) }
    .rodata : { *(.rodata) }
    .data   : { *(.data) }
    .bss    : { *(.bss COMMON) }
    STACK_BASE = .;
    . += 0x10000;
    STACK_TOP = .;
}