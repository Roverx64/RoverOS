[BITS 64]

[GLOBAL ind]
[GLOBAL outd]
[GLOBAL inb]
[GLOBAL outb]

inb:
push rdx
mov edx, edi
mov eax, esi
in al, dx
pop rdx
ret

outb:
push rdx
mov rdx, rdi
mov rax, rsi
out dx, al
pop rdx
ret

ind:
push rdx
mov rdx, qword 0x0
mov rax, qword 0x0
mov dx, di
mov eax, esi
in dword eax, dx
pop rdx
ret

outd:
push rdx
mov dx, di
mov eax, esi
out dx, dword eax
pop rdx
ret