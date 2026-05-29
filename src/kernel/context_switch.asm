; context_switch.asm - Context switching for multitasking
; 
; This code handles the special case of the timer interrupt when
; multitasking is enabled. It performs the context switch.

extern ck_schedule

global ck_timer_context_switch

section .text

; Timer interrupt handler with context switching.
; Called from the timer IRQ stub.
; 
; Stack on entry (from IRQ stub):
;   [esp+36] = EFLAGS
;   [esp+32] = CS
;   [esp+28] = EIP (return address in kernel code)
;   [esp+24] = IRQ number (0)
;   [esp+20] = EDI
;   [esp+16] = ESI
;   [esp+12] = EBP
;   [esp+8]  = ESP (original, before pushad)
;   [esp+4]  = EBX
;   [esp+0]  = EDX, ECX, EAX pushed by pushad
;
; Calling convention: We need to pass a pointer to the saved registers to ck_schedule.

ck_timer_context_switch:
    ; At this point, all general-purpose registers are already saved on stack by pushad.
    ; We need to:
    ; 1. Create a ck_registers structure from the saved state
    ; 2. Call ck_schedule with a pointer to these registers
    ; 3. Restore the returned registers
    ; 4. Return from interrupt
    
    ; Adjust ESP to skip past IRQ number pushed by stub
    add esp, 4              ; Skip IRQ number
    
    ; Now ESP points to EDI (the first register pushed by pushad)
    ; This is exactly the layout of struct ck_registers!
    ; EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    
    ; Pass pointer to registers as argument
    mov eax, esp
    push eax
    call ck_schedule
    
    ; EAX now contains pointer to next task's registers
    mov esp, eax            ; Set ESP to point to next task's registers
    
    ; Add back the IRQ number placeholder (will be popped by add esp, 4)
    sub esp, 4
    mov dword [esp], 0      ; Dummy IRQ number
    
    ; Now restore all registers and return from interrupt
    popad                   ; Restore EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    add esp, 4              ; Pop IRQ number
    iret                    ; Return from interrupt, restoring EIP, CS, EFLAGS
