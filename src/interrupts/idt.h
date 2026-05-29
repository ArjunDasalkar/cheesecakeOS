#ifndef CK_IDT_H
#define CK_IDT_H

#include <stdint.h>

/*
 * IDT = Interrupt Descriptor Table.
 * It is the CPU's table for deciding what code to run when an interrupt happens.
 */
#define CK_IDT_ENTRIES 256

/*
 * Kernel code segment selector (from GRUB GDT).
 * CS is 0x10 in the current environment, so use that for IDT gates.
 */
#define CK_KERNEL_CS 0x10

/*
 * One IDT entry tells the CPU where the interrupt handler lives,
 * which code segment to use, and what privilege/behavior flags to apply.
 */
struct ck_idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

/*
 * lidt needs a pointer to the table and its size.
 * limit = size in bytes minus 1, base = address of the first entry.
 */
struct ck_idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* Load my IDT into the CPU. */
void ck_idt_init(void);

/* Set a single IDT gate (used by exception handlers and IRQ handlers). */
void ck_idt_set_gate(uint8_t vector, uint32_t base, uint16_t selector, uint8_t flags);

#endif