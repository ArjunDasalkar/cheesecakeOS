#ifndef CK_PIC_H
#define CK_PIC_H

#include <stdint.h>

/*
 * PIC = Programmable Interrupt Controller
 * Two 8259 PICs: master (handles IRQs 0-7) and slave (handles IRQs 8-15)
 *
 * By default, IRQs map to vectors 8-15, which conflicts with CPU exceptions.
 * We remap to: Master→32-39, Slave→40-47
 */

#define CK_PIC_MASTER_CMD       0x20
#define CK_PIC_MASTER_DATA      0x21
#define CK_PIC_SLAVE_CMD        0xA0
#define CK_PIC_SLAVE_DATA       0xA1

/*
 * IRQ vector offsets after remapping.
 * IRQ 0 (timer) → vector 32
 * IRQ 1 (keyboard) → vector 33
 * IRQ 8 (RTC on slave) → vector 40
 */
#define CK_IRQ_OFFSET_MASTER    32
#define CK_IRQ_OFFSET_SLAVE     40

/*
 * Standard IRQ numbers.
 */
#define CK_IRQ_TIMER            0
#define CK_IRQ_KEYBOARD         1

/* Initialize and remap the PIC. Call this before enabling interrupts. */
void ck_pic_init(void);

/* Send End-of-Interrupt signal to the PIC. Call this at end of every IRQ handler. */
void ck_pic_send_eoi(uint8_t irq);

#endif
