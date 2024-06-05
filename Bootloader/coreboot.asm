[BITS 16]
org 0x0

;Bootstraps an AP to long mode

;base 0x6000
;   +-------------+
;00h|stack pointer|-->+-------+
;08h|64 Bit GDT   |   |4KB RSP| The 64 bit stack pointer is loaded into the 16 bit stack as the first entry of each core's stack
;10h|64 Bit IDT   |   \/\/ \/\/ (c*4 qwords) where c = core count
;18h|PML4E        |
;   +-------------+

[EXTERN initMTRR]
[EXTERN registerLapic]

section .text

entry:
;=====16 BIT MODE====;
cli
xor al, al
;Clear segment registers
mov ds, al
mov es, al
mov fs, al
mov gs, al
mov ss, al
;Get stack lock
.getLock:
mov ax, stackLock
mov byte cx, [ax]
cmp cx, 0
jne .getLock
;Set lock
mov byte [ax], 0x1
;Load stack pointer
mov dx, 0x6FF8
mov cx, [dx]
mov bp, cx
add cx, 8 ;Offset by 8 to account for the 8 byte rsp
mov sp, cx
;Increment pointer by 4 qwords
add cx, 32
mov word [dx], cx
;Free lock
mov byte [ax], 0x0
;Init prot mode
;Load GDT
mov eax, GDT32P
lgdt [eax]
mov eax, .reload
push eax
retf
.reload:
;Enable PE
mov eax, cr0
or eax, 0x1
mov cr0, eax
;
;=====32 BIT MODE====;
;
[BITS 32]
;Init long mode
;Load 64 bit GDT ptr from 0x6000
mov esi, 0x6000
mov eax, [esi]
lgdt [eax]
mov eax, 0x8
push eax
mov eax, gret
push gret
retfd
.gret:
;Set LME
mov ecx, 0xC0000080
rdmsr
and eax, 1<<8
wrmsr
;Enable PAE
mov eax, cr0
or cr0, 1<<5
;Set PML4E
mov esi, 0x6008
mov eax, [esi]
mov cr3, eax
;Enable long mode
mov eax, cr0
or eax, 1<<31
mov cr0, eax
;
;=====64 BIT MODE====;
;
[BITS 64]
;Load 64 bit stack
pop rax
mov rsp, rax
mov rbp, rax
;Call kernel functions to init processor
;Initilize MTRRs
lea rax, initMTRR
call rax
;Enable lapic
mov rcx, 0x1B
rdmsr
or eax 1<<10
wrmsr
;Get lapic bar
rdmsr
shr eax, 11
shl edx, 11
or eax, edx
push rax
;Get lapic id
add rax, 0x20
mov dword rsi, [rax]
pop rdi
;Register lapic and wait for a task
lea rax, registerLapic
call rax
.endini:
hlt
jmp .endini

section .data

stackLock dq 0 ;Mutex lock

section .rodata

GDT32P:
dw 0x3
dd GDT32
GDT32:
dq 0x0
dq 0x00CF9A000000FFFF
dq 0x00CF92000000FFFF