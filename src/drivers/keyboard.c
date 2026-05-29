#include "keyboard.h"
#include "scancode.h"
#include "../interrupts/irq.h"
#include "../drivers/timer.h"

/*
 * PS/2 keyboard I/O port.
 */
#define CK_KEYBOARD_PORT        0x60

/*
 * Keyboard buffer.
 * Simple circular buffer for scancodes.
 */
#define CK_KEYBOARD_BUFFER_SIZE 256
static uint8_t ck_keyboard_buffer[CK_KEYBOARD_BUFFER_SIZE];
static uint16_t ck_keyboard_head = 0;  /* Write pointer */
static uint16_t ck_keyboard_tail = 0;  /* Read pointer */

/*
 * Shift state tracking.
 */
static uint8_t ck_keyboard_shift_pressed = 0;

/*
 * Key repeat tracking.
 */
#define CK_KEY_REPEAT_INITIAL_DELAY 500  /* ms before repeat starts */
#define CK_KEY_REPEAT_INTERVAL      50   /* ms between repeats */

static uint8_t ck_keyboard_held_key = 0;          /* Current key being held */
static uint32_t ck_keyboard_held_key_time = 0;   /* Timer tick when key was first pressed */
static uint32_t ck_keyboard_last_repeat_time = 0; /* Timer tick of last repeat event */

/*
 * Port I/O helpers.
 */
static inline uint8_t ck_inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/*
 * Keyboard interrupt handler (IRQ 1).
 * Reads the scancode from the keyboard port and stores it in the buffer.
 * Tracks shift state and key repeat.
 */
static void ck_keyboard_irq_handler(uint8_t irq) {
    (void)irq;  /* Unused */
    
    uint8_t scancode = ck_inb(CK_KEYBOARD_PORT);
    
    /* Detect key release (bit 7 set means break code) */
    uint8_t is_release = (scancode >= 0x80);
    
    /* Handle shift state */
    if (scancode == 0x2A || scancode == 0x36) {
        /* Left or right shift pressed */
        ck_keyboard_shift_pressed = 1;
        return;  /* Don't add to buffer */
    } else if (scancode == 0xAA || scancode == 0xB6) {
        /* Left or right shift released */
        ck_keyboard_shift_pressed = 0;
        return;  /* Don't add to buffer */
    }
    
    /* Handle key press / release */
    if (is_release) {
        /* Key released */
        uint8_t press_code = scancode - 0x80;
        if (ck_keyboard_held_key == press_code) {
            ck_keyboard_held_key = 0;  /* No longer holding this key */
        }
        return;  /* Don't add release codes to buffer */
    } else {
        /* Key pressed */
        uint32_t now = ck_timer_get_ticks();
        ck_keyboard_held_key = scancode;
        ck_keyboard_held_key_time = now;
        ck_keyboard_last_repeat_time = now;
        
        /* Add to buffer if not full */
        uint16_t next_head = (ck_keyboard_head + 1) % CK_KEYBOARD_BUFFER_SIZE;
        if (next_head != ck_keyboard_tail) {
            ck_keyboard_buffer[ck_keyboard_head] = scancode;
            ck_keyboard_head = next_head;
        }
        /* If buffer is full, drop the scancode */
    }
}

/*
 * Initialize the keyboard driver.
 */
void ck_keyboard_init(void) {
    /* Register IRQ handler for keyboard (IRQ 1) */
    ck_irq_set_handler(1, ck_keyboard_irq_handler);
}

/*
 * Check if a key is available.
 */
uint8_t ck_keyboard_has_key(void) {
    return (ck_keyboard_head != ck_keyboard_tail) ? 1 : 0;
}

/*
 * Read the next scancode from the buffer.
 * Returns 0 if no key is available.
 */
uint8_t ck_keyboard_read_key(void) {
    if (ck_keyboard_head == ck_keyboard_tail) {
        return 0;  /* Buffer empty */
    }
    
    uint8_t scancode = ck_keyboard_buffer[ck_keyboard_tail];
    ck_keyboard_tail = (ck_keyboard_tail + 1) % CK_KEYBOARD_BUFFER_SIZE;
    
    return scancode;
}

/*
 * Check if shift is currently pressed.
 */
uint8_t ck_keyboard_is_shift_pressed(void) {
    return ck_keyboard_shift_pressed;
}

/*
 * Read the next ASCII character from the buffer.
 * Returns 0 if no key or not printable.
 * Handles key repeat: if a key is held long enough, generates repeat events.
 */
char ck_keyboard_read_char(void) {
    uint8_t scancode = ck_keyboard_read_key();
    
    /* If no key in buffer, check for repeat event */
    if (scancode == 0 && ck_keyboard_held_key != 0) {
        uint32_t now = ck_timer_get_ticks();
        uint32_t elapsed = now - ck_keyboard_held_key_time;
        
        /* Only generate repeat after initial delay */
        if (elapsed >= CK_KEY_REPEAT_INITIAL_DELAY) {
            uint32_t time_since_last_repeat = now - ck_keyboard_last_repeat_time;
            
            /* Generate repeat event if enough time has passed */
            if (time_since_last_repeat >= CK_KEY_REPEAT_INTERVAL) {
                ck_keyboard_last_repeat_time = now;
                scancode = ck_keyboard_held_key;  /* Repeat the held key */
            }
        }
    }
    
    if (scancode == 0) {
        return 0;  /* No key available */
    }
    
    return (char)ck_scancode_to_ascii(scancode, ck_keyboard_shift_pressed);
}
