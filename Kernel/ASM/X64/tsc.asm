[BITS 64]

[GLOBAL checkForTSC]
checkForTSC:
push rbx
mov rax, qword 0x1
cpuid
test rax, 0x10
jz .noTSC
mov rax, 0x1
pop rbx
ret
.noTSC:
mov rax, 0x0
pop rbx
ret