#include "timer.h"
#include "../interrupts/irq.h"
#include "../interrupts/pic.h"

/*
 * PIT (Programmable Interval Timer) I/O ports.
 */
#define CK_PIT_CHANNEL0         0x40    /* Timer 0 data port */
#define CK_PIT_CONTROL          0x43    /* Control/status port */

/*
 * Tick counter.
 * Incremented by the timer ISR.
 */
static uint32_t ck_timer_ticks = 0;

/*
 * Port I/O helpers.
 */
static inline void ck_outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t ck_inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/* Forward declaration for scheduler */
extern struct ck_registers *ck_schedule(struct ck_registers *current_regs);

/*
 * Timer interrupt handler.
 * Called by IRQ 0 every 1 ms (if configured for 1 kHz).
 * 
 * Called from assembly context where all registers are already saved.
 * This handler performs context switching for multitasking.
 */
static void ck_timer_irq_handler(uint8_t irq) {
    (void)irq;  /* Unused */
    ck_timer_ticks++;
    
    /* Scheduler will be invoked from assembly for true context switching */
}

/*
 * Initialize the PIT to generate interrupts at 1 kHz.
 * 
 * PIT runs at ~1.193 MHz (1193182 Hz).
 * To get 1 kHz, we set reload value to 1193182 / 1000 = 1193.
 * 
 * Mode: 0x34 = counter 0, 16-bit access, mode 2 (rate generator)
 */
void ck_timer_init(void) {
    uint16_t reload_value = 1193;  /* 1 kHz */

    /* Set PIT mode: counter 0, 16-bit, mode 2 (rate generator) */
    ck_outb(CK_PIT_CONTROL, 0x34);

    /* Load reload value (low byte then high byte) */
    ck_outb(CK_PIT_CHANNEL0, (uint8_t)(reload_value & 0xFF));
    ck_outb(CK_PIT_CHANNEL0, (uint8_t)((reload_value >> 8) & 0xFF));

    /* Register IRQ handler */
    ck_irq_set_handler(CK_IRQ_TIMER, ck_timer_irq_handler);
}

/*
 * Get the current tick count.
 */
uint32_t ck_timer_get_ticks(void) {
    return ck_timer_ticks;
}
