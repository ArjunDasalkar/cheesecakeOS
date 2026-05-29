; =============================================================================
; boot.asm — CheesecakeOS entry point
; First code that runs when GRUB hands control to the kernel.
; Written in x86 32-bit assembly (NASM syntax).
; =============================================================================

extern ck_main                  ; ck_main is defined in ck_kernel.c

section .multiboot2
align 8

mb2_start:
    dd 0xE85250D6               ; Multiboot2 magic number
    dd 0                        ; Architecture: 0 = 32-bit protected mode x86
    dd mb2_end - mb2_start      ; Header length
    dd 0x100000000 - (0xE85250D6 + 0 + (mb2_end - mb2_start))  ; Checksum

    ; End tag — tells GRUB there are no extra requests
    dw 0                        ; Type: end tag
    dw 0                        ; Flags
    dd 8                        ; Size
mb2_end:

; =============================================================================
; Stack setup
; CPU needs a stack before running C code. C functions use stack for
; local vars, function arguments, return addresses.
; Manually allocate 16KB and point stack pointer to it.
; =============================================================================

section .bss
align 16

ck_stack_bottom:
    resb 16384          ; Reserve 16KB for stack
ck_stack_top:

; =============================================================================
; Kernel entry point
; GRUB jumps here after loading kernel. Setup stack and call C code.
; =============================================================================

section .text
global ck_start

ck_start:
    cli                         ; Keep interrupts off during early init
    mov esp, ck_stack_top       ; Point stack pointer to top of stack
    and esp, 0xFFFFFFF0         ; Align stack to 16 bytes (C calling convention)

    call ck_main                ; Call C kernel function

    ; If ck_main returns, halt the CPU
.hang:
    cli                         ; Disable interrupts
    hlt                         ; Halt CPU
    jmp .hang                   ; If woken up, halt again