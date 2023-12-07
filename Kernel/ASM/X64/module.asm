[BITS 64]

[GLOBAL modSwitch]

;Performs a context switch to a module
;RDI = RIP
;RSI = RSP
;RDX = RBP

modSwitch:
;Save registers per the System-V ABI
push rbx
push r12
push r13
push r14
push r14
push rsp
push rbp
;Switch stacks
mov rsp, rsi
mov rbp, rdx
;Setup rip and call module
push rdi
ret

[GLOBAL testSwitch]
;RDI = cr3
testSwitch:
mov cr3, rdi
mov rdi, qword 0x0
mov rax, qword [rdi]
hlt
ret