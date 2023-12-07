[BITS 64]

[GLOBAL checkHardware]

;Checks for XD support and enables it
;Some hardware, like my dell optiplex, does not enable XD by default even when enabled in UEFI
;Returns a bitmap of supported hardware
;0 = Not supported
;Bit 0 = XD support
checkHardware:
push rbx
push rcx
push rdx
xor rdx, rdx
;Get info
mov rax, 0x80000001
cpuid
xor rax, rax
;Test for XD support
test rdx, 0x100000
jz .unsupported
or rax, 0x1 ;XD is supported
;Enable XD if not already enabled
push rax
xor rax, rax
mov rcx, 0xC0000080
rdmsr
or rax, 0x800
wrmsr
pop rax
.unsupported:
pop rdx
pop rcx
pop rbx
ret