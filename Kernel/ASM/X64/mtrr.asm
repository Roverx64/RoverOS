[BITS 64]

[GLOBAL initMTRR]

;Initilizes MTRRs to known values
;All memory will be WB and the PAT will be used to use other memory types

initMTRR:
;Read MTRRcap register
mov rcx, 0xFE
rdmsr
and rax, qword 0xFF
mov r9, rax ;Save VCNT
;Set WB
mov rax, 0x06060606
mov rdx, rax
;Set fixed ranged MTRRs
;64k MSR
mov rcx, 0x250
wrmsr
;16k MSRs
mov rcx, 0x258
wrmsr
inc rcx
wrmsr
;4k MSRs
mov r10, 8
mov rcx, 0x268
.smsr:
wrmsr
inc rcx
cmp r10, 0
jne .smsr
;Set 1st variable range register to WB and disable the rest
mov rcx, 0x200
xor rdx, rdx
mov rax, qword 0x6
;Set Base
wrmsr
inc rcx
;Set Mask
mov rax, qword 0xFFFFF800
mov rdx, qword 0xFFFFF
wrmsr
inc rcx
;Loop VCNT and disable other MTRRs
xor rax, rax
xor rdx, rdx
mov rax, r9 ;Load VCNT
.vset:
dec r10
;Base
wrmsr
inc rcx
;Mask
wrmsr
inc rcx
cmp r10, 0
jne .vset
;Set PAT WC option
;PA4 = WC
mov rcx, 0x277
rdmsr
and rdx, 0xFFFFFF00
or rdx, 0x01
wrmsr
;Ensure caching is enabled
mov rax, cr0
and rax, 0x9FFFFFFF
mov cr0, rax
ret