#include "irq.h"
#include "idt.h"
#include "pic.h"

/*
 * Forward declarations for assembly stubs.
 * Each stub is named ck_isr_irq_<number>.
 */
extern void ck_isr_irq_0(void);
extern void ck_isr_irq_1(void);
extern void ck_isr_irq_2(void);
extern void ck_isr_irq_3(void);
extern void ck_isr_irq_4(void);
extern void ck_isr_irq_5(void);
extern void ck_isr_irq_6(void);
extern void ck_isr_irq_7(void);
extern void ck_isr_irq_8(void);
extern void ck_isr_irq_9(void);
extern void ck_isr_irq_10(void);
extern void ck_isr_irq_11(void);
extern void ck_isr_irq_12(void);
extern void ck_isr_irq_13(void);
extern void ck_isr_irq_14(void);
extern void ck_isr_irq_15(void);

/*
 * Table of IRQ handler entry points.
 */
static void (*ck_irq_handlers_asm[16])(void) = {
    ck_isr_irq_0,  ck_isr_irq_1,  ck_isr_irq_2,  ck_isr_irq_3,
    ck_isr_irq_4,  ck_isr_irq_5,  ck_isr_irq_6,  ck_isr_irq_7,
    ck_isr_irq_8,  ck_isr_irq_9,  ck_isr_irq_10, ck_isr_irq_11,
    ck_isr_irq_12, ck_isr_irq_13, ck_isr_irq_14, ck_isr_irq_15,
};

/*
 * Table of custom C handlers.
 * Can be set with ck_irq_set_handler().
 */
static ck_irq_handler_t ck_irq_handlers_c[16];

/*
 * Default handler: does nothing.
 */
void ck_irq_default_handler(uint8_t irq) {
    (void)irq;  /* Unused */
}

/*
 * IRQ dispatcher.
 * Called by assembly stub. Calls custom handler and sends EOI.
 */
void ck_irq_handler(uint8_t irq) {
    if (irq < 16 && ck_irq_handlers_c[irq]) {
        ck_irq_handlers_c[irq](irq);
    }
    ck_pic_send_eoi(irq);
}

/*
 * Set a custom handler for an IRQ.
 */
void ck_irq_set_handler(uint8_t irq, ck_irq_handler_t handler) {
    if (irq < 16) {
        ck_irq_handlers_c[irq] = handler;
    }
}

/*
 * Register IRQ handlers in the IDT.
 */
void ck_irq_init(void) {
    uint8_t irq;

    /* Initialize all handlers to default */
    for (irq = 0; irq < 16; ++irq) {
        ck_irq_handlers_c[irq] = ck_irq_default_handler;
    }

    /* Register all IRQs in the IDT (vectors 32-47) */
    for (irq = 0; irq < 16; ++irq) {
        uint32_t vector = (irq < 8) ? (CK_IRQ_OFFSET_MASTER + irq) : (CK_IRQ_OFFSET_SLAVE + (irq - 8));
        ck_idt_set_gate((uint8_t)vector, (uint32_t)ck_irq_handlers_asm[irq], CK_KERNEL_CS, 0x8E);
    }
}
