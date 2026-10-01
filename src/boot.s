; Multiboot v1 header + kernel entry point (i386).
; GRUB looks for the header in the first 8 KiB, loads the kernel at 1 MiB
; (see linker.ld), switches to 32-bit protected mode and jumps to _start with
; eax = multiboot magic and ebx = address of the info structure.
bits 32

MBALIGN     equ 1 << 0          ; align modules on 4 KiB
MEMINFO     equ 1 << 1          ; request the memory map
FLAGS       equ MBALIGN | MEMINFO
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS) ; magic + flags + checksum must be 0

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
global stack_top                ; read by print_k_stack
stack_bottom:
    resb 16384                  ; 16 KiB stack, grows downwards
stack_top:

section .text
global _start
extern main

_start:
    mov     esp, stack_top
    xor     ebp, ebp                ; end of frame chain for a debugger
    push    ebx                 ; multiboot info pointer
    push    eax                 ; multiboot magic
    call    main
    cli                         ; if main returns: halt the CPU
.hang:
    hlt
    jmp     .hang
