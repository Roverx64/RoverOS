ENTRY(kmain)
SECTIONS{
  . = 0xC0000000;
  _kernel_start = .;
  .text : ALIGN(0x200000){*(.text)}
  .data : ALIGN(0x200000){*(.data) *(.rodata)}
  .bss : ALIGN(0x200000){*(.bss)}
  kernel_end = .; _kernel_end = .; __kernel_end = .;
}
