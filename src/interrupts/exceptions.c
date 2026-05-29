#include "exceptions.h"
#include "idt.h"

/*
 * Forward declarations for the assembly stubs.
 * Each stub is named ck_isr_exception_<vector>.
 */
extern void ck_isr_exception_0(void);
extern void ck_isr_exception_1(void);
extern void ck_isr_exception_2(void);
extern void ck_isr_exception_3(void);
extern void ck_isr_exception_4(void);
extern void ck_isr_exception_5(void);
extern void ck_isr_exception_6(void);
extern void ck_isr_exception_7(void);
extern void ck_isr_exception_8(void);
extern void ck_isr_exception_9(void);
extern void ck_isr_exception_10(void);
extern void ck_isr_exception_11(void);
extern void ck_isr_exception_12(void);
extern void ck_isr_exception_13(void);
extern void ck_isr_exception_14(void);
extern void ck_isr_exception_15(void);
extern void ck_isr_exception_16(void);
extern void ck_isr_exception_17(void);
extern void ck_isr_exception_18(void);
extern void ck_isr_exception_19(void);
extern void ck_isr_exception_20(void);
extern void ck_isr_exception_21(void);
extern void ck_isr_exception_22(void);
extern void ck_isr_exception_23(void);
extern void ck_isr_exception_24(void);
extern void ck_isr_exception_25(void);
extern void ck_isr_exception_26(void);
extern void ck_isr_exception_27(void);
extern void ck_isr_exception_28(void);
extern void ck_isr_exception_29(void);
extern void ck_isr_exception_30(void);
extern void ck_isr_exception_31(void);

/*
 * Table of exception handlers.
 * Index = vector number.
 */
static void (*ck_exception_handlers[32])(void) = {
    ck_isr_exception_0,  ck_isr_exception_1,  ck_isr_exception_2,  ck_isr_exception_3,
    ck_isr_exception_4,  ck_isr_exception_5,  ck_isr_exception_6,  ck_isr_exception_7,
    ck_isr_exception_8,  ck_isr_exception_9,  ck_isr_exception_10, ck_isr_exception_11,
    ck_isr_exception_12, ck_isr_exception_13, ck_isr_exception_14, ck_isr_exception_15,
    ck_isr_exception_16, ck_isr_exception_17, ck_isr_exception_18, ck_isr_exception_19,
    ck_isr_exception_20, ck_isr_exception_21, ck_isr_exception_22, ck_isr_exception_23,
    ck_isr_exception_24, ck_isr_exception_25, ck_isr_exception_26, ck_isr_exception_27,
    ck_isr_exception_28, ck_isr_exception_29, ck_isr_exception_30, ck_isr_exception_31,
};

/*
 * Exception names for debugging.
 */
static const char *ck_exception_names[32] = {
    "Divide By Zero",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved (15)",
    "Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point",
    "Reserved (20)", "Reserved (21)", "Reserved (22)", "Reserved (23)",
    "Reserved (24)", "Reserved (25)", "Reserved (26)", "Reserved (27)",
    "Reserved (28)", "Reserved (29)", "Reserved (30)", "Reserved (31)",
};

/*
 * Minimal VGA printing for exception messages.
 * (We'll expand this or use the main kernel's VGA driver later.)
 */
static void ck_vga_write_char(char c) {
    static uint16_t cursor = 0;
    volatile uint16_t *vga = (volatile uint16_t *)0xB8000;
    if (cursor >= 80 * 25) return;  /* Don't overflow */
    vga[cursor++] = (0x0C << 8) | (unsigned char)c;  /* Red text */
}

static void ck_vga_write_string(const char *s) {
    while (*s) {
        ck_vga_write_char(*s++);
    }
}

/*
 * Exception dispatcher.
 * Called by the assembly stub with the exception frame.
 * vector = exception number (0-31).
 */
void ck_exception_handler(uint32_t vector, struct ck_exception_frame *frame) {
    /* Print exception name */
    ck_vga_write_string("\n[EXC] ");
    ck_vga_write_string(ck_exception_names[vector]);
    
    /* Print error code if present */
    if (vector == 8 || vector == 10 || vector == 11 || vector == 12 || vector == 13 || vector == 14 || vector == 17) {
        ck_vga_write_string(" (err=");
        /* Print error code in hex (simplified) */
        uint32_t err = frame->error_code;
        for (int i = 0; i < 8; i++) {
            int nibble = (err >> (28 - i * 4)) & 0xF;
            ck_vga_write_char("0123456789ABCDEF"[nibble]);
        }
        ck_vga_write_char(')');
    }
    
    ck_vga_write_string("\nHalting.");
    
    /* Halt */
    __asm__("cli");
    __asm__("hlt");
}

/*
 * Register all exception handlers in the IDT.
 * This is called during kernel startup.
 */
void ck_exceptions_init(void) {
    uint8_t vector;
    
    /* Register each exception vector in the IDT */
    for (vector = 0; vector < 32; ++vector) {
        ck_idt_set_gate(vector, (uint32_t)ck_exception_handlers[vector], CK_KERNEL_CS, 0x8E);
    }
}
