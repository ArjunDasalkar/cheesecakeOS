#ifndef CK_IRQ_H
#define CK_IRQ_H

#include <stdint.h>

/*
 * IRQ handler prototype.
 * Called with IRQ number.
 */
typedef void (*ck_irq_handler_t)(uint8_t irq);

/* Initialize IRQ handlers and register them in the IDT. */
void ck_irq_init(void);

/* Register a custom handler for a specific IRQ. */
void ck_irq_set_handler(uint8_t irq, ck_irq_handler_t handler);

/* Default IRQ handler (does nothing). */
void ck_irq_default_handler(uint8_t irq);

#endif
