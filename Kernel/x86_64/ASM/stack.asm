[BITS 64]

section .text

[GLOBAL loadStack]

loadStack:
cli
pop r12 ;Return pointer
pop r13 ;Kinf pointer
;mov rsp, rdi
;mov rbp, rdi
push r13
push r12
ret