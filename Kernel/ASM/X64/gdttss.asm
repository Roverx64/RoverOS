[BITS 64]

section .text

[GLOBAL loadGDT]

loadGDT:
lgdt [rdi]
push 0x8
lea r13, [rel .reload]
push qword r13
retfq
.reload:
mov ax, 0x0
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
ret

[GLOBAL loadTSS]

loadTSS:
ltr di
ret


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

[GLOBAL taskSwitchEntry]
[EXTERN switchTask]
taskSwitchEntry:
pushall
call switchTask
ret

[GLOBAL testSyscall]
testSyscall:
mov rdi, 0x0
syscall
ret