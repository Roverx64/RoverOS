ENTRY(kmain)
SECTIONS{
  . = 0xC0000000;
  _kernel_start = .;
  .text : {
    code = .; _code = .; __code = .;
    *(.text)
    . = ALIGN(4096);
  }
  .data : {
    data = .; _data = .; __data = .;
    *(.data)
    *(.rodata)
    . = ALIGN(4096);
  }
  .bss : {
    bss = .; _bss = .; __bss = .;
    *(.bss)
    . = ALIGN(4096);
  }
  linker_end = .; _linker_end = .; __linker_end = .;
}
