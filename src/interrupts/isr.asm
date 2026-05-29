global ck_isr_default_stub

section .text

; Default interrupt stub.
; If any interrupt hits before adding a real handler, the CPU lands here.
ck_isr_default_stub:
    cli                 ; Keep interrupts off so nothing new can interrupt this stub
.hang:
    hlt                 ; Sleep until the CPU is interrupted again
    jmp .hang           ; If it wakes up, go back to sleep
