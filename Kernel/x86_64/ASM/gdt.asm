[BITS 64]

section .text

[GLOBAL loadGDT]

loadGDT:
lgdt [rdi]
push 0x8
lea r13, [rel .reload]
push qword r13
retfq
.reload:
mov ax, 0x0
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
ret