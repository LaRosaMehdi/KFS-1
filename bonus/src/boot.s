; Multiboot v1 + pile + appel de main.
bits 32

MBALIGN     equ 1 << 0
MEMINFO     equ 1 << 1
FLAGS       equ MBALIGN | MEMINFO
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start
extern main

_start:
    mov     esp, stack_top
    xor     ebp, ebp
    push    ebx
    push    eax
    call    main
    cli
.hang:
    hlt
    jmp     .hang
