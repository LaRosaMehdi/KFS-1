; CPU code that can't be written in C:
; - gdt_flush / idt_flush: load the tables (lgdt / lidt);
; - one stub per exception (isr0-31) and per IRQ (irq0-15), which pushes an
;   error code (0 if the CPU doesn't provide one) and the vector number, then
;   jumps to a common path that saves the registers and calls into C.
; Tables isr_stubs / irq_stubs: stub addresses, read by idt.c.

bits 32

; void gdt_flush(uint32_t gdt_ptr)
; After lgdt, the segment registers still hold the old values: reload them
; with 0x10 (data) and 0x18 (stack), and reload cs (0x08, code) with a far
; jump.
global gdt_flush
gdt_flush:
    mov     eax, [esp + 4]
    lgdt    [eax]
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ax, 0x18
    mov     ss, ax
    jmp     0x08:.reload
.reload:
    ret

; void idt_flush(uint32_t idt_ptr)
global idt_flush
idt_flush:
    mov     eax, [esp + 4]
    lidt    [eax]
    ret

; 8, 10-14 and 17 already push an error code.
%macro ISR 1
global isr%1
isr%1:
%if %1 == 8 || %1 == 10 || %1 == 11 || %1 == 12 || %1 == 13 || %1 == 14 || %1 == 17
    push    %1
%else
    push    0
    push    %1
%endif
    jmp     isr_common
%endmacro

%macro IRQ 1
global irq%1
irq%1:
    push    0
    push    (%1 + 32)
    jmp     irq_common
%endmacro

%assign i 0
%rep 32
    ISR i
%assign i i+1
%endrep

%assign i 0
%rep 16
    IRQ i
%assign i i+1
%endrep

global isr_stubs
isr_stubs:
%assign i 0
%rep 32
    dd isr%+i
%assign i i+1
%endrep

global irq_stubs
irq_stubs:
%assign i 0
%rep 16
    dd irq%+i
%assign i i+1
%endrep

; Common path: saves the registers and ds, switches to kernel segments, calls
; the C handler with a pointer to the stack (struct registers), restores
; everything, drops vector + error code (8 bytes) and returns with iret.
%macro INT_STUB 1
    pusha
    xor     eax, eax
    mov     ax, ds
    push    eax
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    push    esp
    call    %1
    add     esp, 4
    pop     eax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    popa
    add     esp, 8
    iret
%endmacro

extern isr_handler
isr_common:
    INT_STUB isr_handler

extern irq_handler
irq_common:
    INT_STUB irq_handler
