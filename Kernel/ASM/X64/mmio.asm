[BITS 64]

[GLOBAL writeMMIO]

;RDI = bar
;RSI = Offset
;RDX = Value
;RCX = Size

writeMMIO:
;Set BAR
add rdi, rsi
;Test for Qword
cmp rcx, 0
je .qwrite
;Test for atomic qword
cmp rcx, 4
je .qwrite
;Test for dword
cmp rcx, 1
je .dwrite
;Test for word
cmp rcx, 2
je .wwrite
;Test for byte
cmp rcx, 3
je .bwrite
jmp .none
;MMIO read functions
.qwrite:
mov [rdi], qword rdx
ret
.dwrite:
mov [rdi], dword edx
ret
.wwrite:
mov [rdi], word dx
ret
.bwrite:
mov [rdi], byte dl
ret
.none:
mov rax, 0x0
ret

;RDI = bar
;RSI = Offset
;RDX = Size
;RDX: 0 = Qword
;RDX: 1 = Dword
;RDX: 2 = word
;RDX: 3 = byte
;NOTE: Soon to be replaced with GCC inline assembly

[GLOBAL readMMIO]
readMMIO:
;Set BAR
mov qword rax, 0x0
add rsi, rdi
;Test for Qword
cmp rdx, 0
je .qread
;Test for atomic qword
cmp rdx, 4
je .qreada
;Test for dword
cmp rdx, 1
je .dread
;Test for word
cmp rdx, 2
je .wread
;Test for byte
cmp rdx, 3
je .bread
jmp .none
;MMIO read functions
.qreada:
mov rax, [rsi]
ret
.qread:
mov dword eax, [rsi]
add rsi, 4
mov dword ecx, [rsi]
shl rcx, 0x4
or rax, rcx
ret
.dread:
mov dword eax, [rsi]
ret
.wread:
mov word ax, [rsi]
ret
.bread:
mov byte al, [rsi]
ret
.none:
mov rax, 0x0
ret