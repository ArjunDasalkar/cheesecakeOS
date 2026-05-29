; irq.asm - Hardware interrupt handlers (vectors 32-47, IRQs 0-15)

extern ck_irq_handler

section .text

; Macro to create an IRQ stub.
; Hardware IRQs don't push error codes, so we just push the IRQ number
; and call the common handler.
%macro CK_ISR_IRQ 1
global ck_isr_irq_%1
ck_isr_irq_%1:
    push dword %1           ; Push IRQ number
    jmp ck_irq_common
%endmacro

; Common handler: save registers and call C dispatcher.
ck_irq_common:
    pushad                  ; Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    ; IRQ number is at [esp + 32] after pushad
    mov eax, [esp + 32]
    push esp                ; arg1: pointer to registers (not used yet, kept for consistency)
    push eax                ; arg2: IRQ number
    call ck_irq_handler
    add esp, 8              ; Pop arguments
    
    popad                   ; Restore registers
    add esp, 4              ; Pop IRQ number
    iret                    ; Return from interrupt

; IRQ stubs for all 16 IRQs
CK_ISR_IRQ 0    ; Timer
CK_ISR_IRQ 1    ; Keyboard
CK_ISR_IRQ 2    ; Cascade (slave PIC)
CK_ISR_IRQ 3    ; Serial (COM2)
CK_ISR_IRQ 4    ; Serial (COM1)
CK_ISR_IRQ 5    ; Parallel (LPT2)
CK_ISR_IRQ 6    ; Floppy disk
CK_ISR_IRQ 7    ; Parallel (LPT1)
CK_ISR_IRQ 8    ; Real-time clock
CK_ISR_IRQ 9    ; Redirect to IRQ2 (cascade)
CK_ISR_IRQ 10   ; Reserved
CK_ISR_IRQ 11   ; Reserved
CK_ISR_IRQ 12   ; Mouse / PS/2
CK_ISR_IRQ 13   ; Coprocessor
CK_ISR_IRQ 14   ; IDE primary
CK_ISR_IRQ 15   ; IDE secondary
