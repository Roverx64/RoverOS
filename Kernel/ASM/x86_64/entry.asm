[BITS 64]
[GLOBAL kentry]
[EXTERN kmain]
[EXTERN __stackBottom]

;; Kernel entry point
kentry:
;Set stack
mov rsp, __stackBottom
;NULL stack frame
xor rax, rax
push rax
push rax
mov rbp, rsp
;Call kernel
call kmain
jmp $