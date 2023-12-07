ENTRY(kentry)
SECTIONS{
  . = 0xFFFFFFFF80000000;
  .text : ALIGN(0x1000){*(.text)}
  .data : ALIGN(0x1000){*(.data) *(.rodata)}
  .bss : ALIGN(0x1000){*(.bss)}
}
