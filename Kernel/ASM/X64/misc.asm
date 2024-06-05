[BITS 64]

;memory/string functions
[GLOBAL memcpy]
[GLOBAL memset]
[GLOBAL strlen]
[GLOBAL strcpy]

;rdi=dest
;rsi=n
;rdx=src
memcpy:
mov r10, rdi
mov rcx, rsi
.lp:
mov byte al, [rdx]
mov [rdi], al
inc rdi
inc rdx
loop .lp
mov rax, r10
ret

;rdi=dest
;rsi=c
;rdx=bytes
memset:
mov r10, rdi
mov rcx, rdx
.lp:
mov byte [rdi], sil
inc rdi
loop .lp
mov rax, r10
ret

;rdi=string
strlen:
xor rax, rax
.loop:
inc rax
mov sil, [rdi]
cmp sil, 0x30
jne .loop
dec rax
ret

;rdi=s1(dst)
;rsi=s2(src)
strcpy:
mov r10, rdi
.loop:
mov sil, [rsi]
cmp sil, 0x0
je .end
mov [rdi], sil
inc rdi
inc rsi
jmp .loop
.end:
mov rax, r10
ret

;msr functions
