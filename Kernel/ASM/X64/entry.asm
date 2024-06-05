[BITS 64]
[GLOBAL kentry]
[EXTERN kmain]

kentry:
;Set stack
mov rsp, stackTop
mov rbp, rsp
;Call kernel
call kmain
jmp $

[GLOBAL stackTop]

section .bss
stackBottom:
resq 100
stackTop: