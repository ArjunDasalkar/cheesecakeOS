#include "pic.h"

/*
 * Send a byte to an I/O port.
 */
static inline void ck_outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/*
 * Read a byte from an I/O port.
 */
static inline uint8_t ck_inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/*
 * Small delay (I/O port reads act as delays).
 * Required between PIC commands.
 */
static inline void ck_io_wait(void) {
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

/*
 * PIC remapping.
 * 
 * The process:
 * 1. Send ICW1 (0x11): begin initialization
 * 2. Send ICW2 (offset): set interrupt vector offset
 * 3. Send ICW3: cascade mode (master ↔ slave)
 * 4. Send ICW4 (0x01): x86 mode
 * 5. Send OCW1: interrupt mask (which IRQs to enable)
 */
void ck_pic_init(void) {
    uint8_t master_mask, slave_mask;

    /* Save current interrupt masks. */
    master_mask = ck_inb(CK_PIC_MASTER_DATA);
    slave_mask = ck_inb(CK_PIC_SLAVE_DATA);

    /* Mask all interrupts during remap. */
    ck_outb(CK_PIC_MASTER_DATA, 0xFF);
    ck_outb(CK_PIC_SLAVE_DATA, 0xFF);

    /* ICW1: start initialization, edge-triggered mode */
    ck_outb(CK_PIC_MASTER_CMD, 0x11);
    ck_io_wait();
    ck_outb(CK_PIC_SLAVE_CMD, 0x11);
    ck_io_wait();

    /* ICW2: set interrupt vector offsets */
    ck_outb(CK_PIC_MASTER_DATA, CK_IRQ_OFFSET_MASTER);  /* Master: vectors 32-39 */
    ck_io_wait();
    ck_outb(CK_PIC_SLAVE_DATA, CK_IRQ_OFFSET_SLAVE);    /* Slave: vectors 40-47 */
    ck_io_wait();

    /* ICW3: configure cascading (master on IRQ2, slave on master line 2) */
    ck_outb(CK_PIC_MASTER_DATA, 0x04);  /* Master: slave at line 2 (1 << 2) */
    ck_io_wait();
    ck_outb(CK_PIC_SLAVE_DATA, 0x02);   /* Slave: connected to line 2 */
    ck_io_wait();

    /* ICW4: x86 mode */
    ck_outb(CK_PIC_MASTER_DATA, 0x01);
    ck_io_wait();
    ck_outb(CK_PIC_SLAVE_DATA, 0x01);
    ck_io_wait();

    /*
     * Unmask IRQ0 (timer), IRQ1 (keyboard), and IRQ2 (cascade to slave).
     * Everything else stays masked until a driver enables it.
     */
    master_mask &= (uint8_t)~0x07;  /* Clear bits 0,1,2 */

    ck_outb(CK_PIC_MASTER_DATA, master_mask);
    ck_outb(CK_PIC_SLAVE_DATA, slave_mask);
}

/*
 * Send End-of-Interrupt to the PIC.
 * Must be called after handling an IRQ to signal the PIC that we're done.
 */
void ck_pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        /* If IRQ is on slave, send EOI to slave first, then master */
        ck_outb(CK_PIC_SLAVE_CMD, 0x20);
    }
    /* Always send EOI to master */
    ck_outb(CK_PIC_MASTER_CMD, 0x20);
}
