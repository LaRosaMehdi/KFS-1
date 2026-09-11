; lgdt/lidt et stubs ISR/IRQ. Tables isr_stubs / irq_stubs pour idt.c.

bits 32

global gdt_flush
gdt_flush:
    mov     eax, [esp + 4]
    lgdt    [eax]
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    jmp     0x08:.reload
.reload:
    ret

global idt_flush
idt_flush:
    mov     eax, [esp + 4]
    lidt    [eax]
    ret

; 8, 10-14, 17 poussent deja un code d'erreur.
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
