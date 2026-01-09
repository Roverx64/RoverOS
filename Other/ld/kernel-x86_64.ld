ENTRY(kentry)
SECTIONS{
  . = 0xFFFFFFFF80000000;
  __kernelStart = .;
  .text : ALIGN(0x1000){*(.text)}
  .data : ALIGN(0x1000){*(.data) *(.rodata)}
  .bss : ALIGN(0x1000){*(.bss) *(COMMON)}
  . = ALIGN(0x1000);
  __kernelEnd = .;
  . += 0x1000;
  .stack : ALIGN(0x1000){
    __stackTop = .;
    . += 0x1000;
    __stackBottom = .;
  }
  . += 0x1000;
  .initalHeap : ALIGN(0x1000){
    __heapStart = .;
    . += 0x8000;
    __heapEnd = .;
  }
}
