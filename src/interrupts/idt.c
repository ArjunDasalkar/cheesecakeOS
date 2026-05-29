#include "idt.h"

/*
 * Default handler from assembly.
 * Right now every interrupt points here, so the CPU just stops safely.
 */
extern void ck_isr_default_stub(void);

/*
 * My IDT table in memory.
 * 256 entries because x86 uses vectors 0-255.
 */
static struct ck_idt_entry ck_idt[CK_IDT_ENTRIES];

/*
 * The small structure the CPU wants for lidt.
 * It tells the CPU where the table is and how big it is.
 */
static struct ck_idt_ptr ck_idt_desc;

/*
 * Fill one IDT slot.
 * vector = interrupt number, base = handler address,
 * selector = code segment, flags = gate settings.
 */
void ck_idt_set_gate(uint8_t vector, uint32_t base, uint16_t selector, uint8_t flags) {
    ck_idt[vector].base_low = (uint16_t)(base & 0xFFFF);
    ck_idt[vector].base_high = (uint16_t)((base >> 16) & 0xFFFF);
    ck_idt[vector].selector = selector;
    ck_idt[vector].always0 = 0;
    ck_idt[vector].flags = flags;
}

/*
 * Load the IDT register.
 * After this, the CPU knows where to look when an interrupt fires.
 */
static inline void ck_lidt(struct ck_idt_ptr *idt_ptr) {
    __asm__ volatile ("lidt (%0)" : : "r" (idt_ptr));
}

/*
 * Build a full IDT table.
 * For now, every vector goes to the same safe stub handler.
 */
void ck_idt_init(void) {
    uint16_t index;

    for (index = 0; index < CK_IDT_ENTRIES; ++index) {
        ck_idt_set_gate((uint8_t)index, (uint32_t)ck_isr_default_stub, CK_KERNEL_CS, 0x8E);
    }

    /* Size of the whole table minus 1, as required by lidt. */
    ck_idt_desc.limit = (uint16_t)(sizeof(struct ck_idt_entry) * CK_IDT_ENTRIES - 1);
    /* Address of the first entry in the table. */
    ck_idt_desc.base = (uint32_t)&ck_idt[0];

    /* Tell CPU about the new IDT. */
    ck_lidt(&ck_idt_desc);
}
