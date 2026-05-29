; exceptions.asm - CPU exception handlers (vectors 0-31)
; Each exception has a small stub that saves registers and calls a C handler.

extern ck_exception_handler

; Macro to create an exception stub WITHOUT an error code.
; The CPU doesn't push an error code, so we push a dummy 0.
%macro CK_ISR_NO_ERROR 1
global ck_isr_exception_%1
ck_isr_exception_%1:
    push dword 0            ; Push dummy error code (no error from CPU)
    push dword %1           ; Push exception vector number
    jmp ck_exception_common
%endmacro

; Macro to create an exception stub WITH an error code.
; The CPU already pushed the error code, so we just push the vector.
%macro CK_ISR_WITH_ERROR 1
global ck_isr_exception_%1
ck_isr_exception_%1:
    push dword %1           ; Push exception vector number
    jmp ck_exception_common
%endmacro

section .text

; Common handler: save all registers and call the C dispatcher.
ck_exception_common:
    ; The stack at this point:
    ;   [esp+4] = exception vector
    ;   [esp]   = error code (or 0)
    ;
    ; We're about to push registers, then call the C handler with
    ; a pointer to the exception frame.

    pushad                  ; Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    
    ; Now the stack is:
    ;   [esp+32] = exception vector
    ;   [esp+36] = error code (or 0)
    ;   [esp]    = registers (EDI, ESI, EBP, ESP_old, EBX, EDX, ECX, EAX)
    
    ; Move the exception vector from [esp+32] to a general register
    mov eax, [esp + 32]     ; EAX = vector
    
    ; Call the C handler with a pointer to the exception frame
    ; The frame starts at ESP (where the registers are)
    push esp                ; arg1: pointer to exception frame
    push eax                ; arg2: exception vector
    call ck_exception_handler
    add esp, 8              ; Pop arguments
    
    popad                   ; Restore all registers
    add esp, 8              ; Pop error code and vector
    iret                    ; Return from interrupt (restore EIP, CS, EFLAGS)

; Exception stubs for 0-31
; Vectors 0-7, 16-19: no error code
CK_ISR_NO_ERROR 0   ; Divide by zero
CK_ISR_NO_ERROR 1   ; Debug
CK_ISR_NO_ERROR 2   ; NMI
CK_ISR_NO_ERROR 3   ; Breakpoint
CK_ISR_NO_ERROR 4   ; Overflow
CK_ISR_NO_ERROR 5   ; Bound range
CK_ISR_NO_ERROR 6   ; Invalid opcode
CK_ISR_NO_ERROR 7   ; Device not available

; Vectors 8, 10-14, 17: error code pushed by CPU
CK_ISR_WITH_ERROR 8   ; Double fault
CK_ISR_NO_ERROR 9     ; Coprocessor overrun (no error)
CK_ISR_WITH_ERROR 10  ; Invalid TSS
CK_ISR_WITH_ERROR 11  ; Segment not present
CK_ISR_WITH_ERROR 12  ; Stack segment
CK_ISR_WITH_ERROR 13  ; General protection
CK_ISR_WITH_ERROR 14  ; Page fault
CK_ISR_NO_ERROR 15    ; Reserved (no error)
CK_ISR_NO_ERROR 16    ; Floating point
CK_ISR_WITH_ERROR 17  ; Alignment check
CK_ISR_NO_ERROR 18    ; Machine check
CK_ISR_NO_ERROR 19    ; SIMD FP

; Vectors 20-31: reserved (no error)
CK_ISR_NO_ERROR 20
CK_ISR_NO_ERROR 21
CK_ISR_NO_ERROR 22
CK_ISR_NO_ERROR 23
CK_ISR_NO_ERROR 24
CK_ISR_NO_ERROR 25
CK_ISR_NO_ERROR 26
CK_ISR_NO_ERROR 27
CK_ISR_NO_ERROR 28
CK_ISR_NO_ERROR 29
CK_ISR_NO_ERROR 30
CK_ISR_NO_ERROR 31
