#ifndef CK_EXCEPTIONS_H
#define CK_EXCEPTIONS_H

#include <stdint.h>

/*
 * Exception vector numbers (0-31).
 * These are CPU exceptions, not hardware IRQs.
 */
#define CK_EXC_DIVIDE_BY_ZERO       0
#define CK_EXC_DEBUG                1
#define CK_EXC_NMI                  2
#define CK_EXC_BREAKPOINT           3
#define CK_EXC_OVERFLOW             4
#define CK_EXC_BOUND_RANGE          5
#define CK_EXC_INVALID_OPCODE       6
#define CK_EXC_DEVICE_NOT_AVAILABLE 7
#define CK_EXC_DOUBLE_FAULT         8
#define CK_EXC_COPROC_OVERRUN       9
#define CK_EXC_INVALID_TSS          10
#define CK_EXC_SEGMENT_NOT_PRESENT  11
#define CK_EXC_STACK_SEGMENT        12
#define CK_EXC_GENERAL_PROTECTION   13
#define CK_EXC_PAGE_FAULT           14
/* 15 reserved */
#define CK_EXC_FLOATING_POINT       16
#define CK_EXC_ALIGNMENT_CHECK      17
#define CK_EXC_MACHINE_CHECK        18
#define CK_EXC_SIMD_FP              19
/* 20-31 reserved */

/*
 * Registers saved by the exception stub.
 * Pushed by the ISR in order: EAX, ECX, EDX, EBX, (original ESP), EBP, ESI, EDI.
 */
struct ck_registers {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
};

/*
 * Exception frame pushed by CPU + our ISR.
 * The CPU pushes: EIP, CS, EFLAGS, and (sometimes) error_code.
 * Our ISR pushes: the struct ck_registers above.
 */
struct ck_exception_frame {
    struct ck_registers regs;
    uint32_t error_code;      /* 0 if no error code from CPU */
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};

/* Initialize exception handlers and register them in the IDT. */
void ck_exceptions_init(void);

#endif
