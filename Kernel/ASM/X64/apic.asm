[BITS 64]

[GLOBAL hasX2APIC]
[GLOBAL enableX2APIC]
[GLOBAL enableLapic]
[GLOBAL getLapicBAR]

section .text

hasX2APIC:
push rcx
push r8
mov rax, 0x1
cpuid
mov r8, 0x1
shl r8, 21
test r8,rcx
jnz .hasApic
mov rax, 0x0
jmp end
.hasApic:
mov rax, 0x1
end:
pop r8
pop rcx
ret

enableX2APIC:
push r8
push rax
push rdx
push rcx
mov rcx, 0x01B
rdmsr
mov r8, qword 0x1
shl r8, 10
or rax, r8
wrmsr
pop rcx
pop rdx
pop rax
pop r8
ret

[GLOBAL enableLapic]

enableLapic:
mov rcx, qword 0x1B
rdmsr
push rdx
mov edx, 0x1
shl edx, 11
or eax, edx
pop rdx
wrmsr
ret

getLapicBAR:
push r8
push rcx
mov rax, qword 0x0
mov rcx, 0x01B
rdmsr
and rax, qword 0xFFFFFFFF000
pop rcx
pop r8
ret

[GLOBAL disablePIC]

disablePIC:
mov al, 0xFF
out byte 0xa1, al
out byte 0x21, al
ret

[GLOBAL hasDeadline]
hasDeadline:
mov rax, qword 0x1
cpuid
mov rax, qword 0x0
test rcx, 0x1000000
jne .noDeadline
mov rax, 0x1
.noDeadline:
ret