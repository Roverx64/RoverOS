# AArch64 mode
.section ".text"
.extern bootstrap
.extern __stackTop
.extern __bootmem
.extern __memsize
.extern __loaderaddr
.extern __loadersize
.global bootstrapEntry

bootstrapEntry:
ldr SP, =__stackTop
ldr R1, =__bootmem
ldr R2, =__memsize
ldr R3, =__loaderaddr
ldr R4, =__loadersize
bl bootstrap
# Halt and loop forever
lp:
wfi
b lp
