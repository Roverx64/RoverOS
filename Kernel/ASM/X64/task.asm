[BITS 64]

[GLOBAL initSyscall]
[GLOBAL syscallEntryPrologue]
[GLOBAL switchTask]

%macro pushall 0
  push rax
  push rbx
  push rcx
  push rdx
  push rsi
  push rdi
  push rbp
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15
%endmacro

%macro popall 0
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rbp
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax
%endmacro

;RDI = base
initSyscall:
push rdi
;Enable syscalls
mov rcx, 0xC0000080 ;EFER
rdmsr
or eax, 0x1
wrmsr
ret
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
ret

;rdi=task's rsp
;rsi=task's cr3
switchTask:
;Save SysV registers
push rbx
push rbp
push r12
push r13
push r14
push r15
;Load task's stack
mov rsp, rdi
;Change CR3 if needed
mov rax, cr3
cmp rax, rsi
je .nxt ;No TLB flush
mov cr3, rsi
.nxt:
;Load SysV registers
pop r15
pop r14
pop r13
pop r12
pop rbp
pop rbx
;Return and continue executing
iretq

[EXTERN nextTask]
[GLOBAL contextSwitch]

;Task switch prologue
contextSwitch:
;Save process state
pushall
;Call C code
lea rbx, nextTask
call rbx
ud2

syscallEntryPrologue:
jmp $