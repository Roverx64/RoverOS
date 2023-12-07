[BITS 64]

[EXTERN syscallEntry]
[GLOBAL syscallEntryPrologue]
syscallEntryPrologue:
call syscallEntry
sysret


[GLOBAL enableSyscall]
enableSyscall:
mov rcx, 0xC0000080 ;EFER
rdmsr
or eax, 0x1
wrmsr
ret

;RDI = base
[GLOBAL setSyscallEntry]
setSyscallEntry:
push rax
push rcx
push rdx
push rdi
;Set LSTAR
mov rcx, 0xC0000082
and rdi, 0xFFFFFFFF
mov rax, rdi
pop rdi
shr rdi, 32
mov rdx, rdi
wrmsr
;Set CSTAR
mov rcx, 0xC0000081
rdmsr
;Set CS
mov rdx, 0x00180008
wrmsr
pop rdx
pop rcx
pop rax
ret

;0x18

;RDI = cr3
;RSI = rbp
;RDX = entry
[GLOBAL testRing3]
testRing3:
push rsp
push rbp
push rax
pushf
mov r11, 0x202
mov rsp, rsi
mov rbp, rsp
mov rcx, rdx
o64 sysret