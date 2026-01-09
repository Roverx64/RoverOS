[BITS 64]

;memory/string functions
[GLOBAL memcpy]
[GLOBAL memset]
[GLOBAL strlen]
[GLOBAL strcpy]
[GLOBAL strcmp]

;rdi=dest
;rsi=n
;rdx=src
;; Typical C memcpy
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
;; Typical C memset
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
;; Typical C strlen
strlen:
xor rax, rax
slenLoop:
inc rax
cmp byte [rdi], 0x0
je .end
inc rdi
jmp slenLoop
.end:
dec rax
ret

;rdi=s1(dst)
;rsi=s2(src)
;; Typical C strcpy
strcpy:
mov r10, rdi
scpyLoop:
cmp byte [rsi], 0x0
je .end
movsb
jmp scpyLoop
.end:
movsb
mov rax, r10
ret

;rdi=s1
;rsi=s2
;rax = s1 byte
;rcx = s2 byte
strcmp:
xor rax, rax
xor rcx, rcx
scmpLoop:
mov byte al, [rdi]
mov byte cl, [rsi]
;Mask to single byte
inc rdi
inc rsi
cmp al, cl
jnz .end
cmp al, 0x0
jnz scmpLoop
.end:
cmp al, cl
jc .lcl
.lal:
sub rax, rcx
ret
.lcl:
sub rcx, rax
mov rax, rcx
ret

;msr functions
