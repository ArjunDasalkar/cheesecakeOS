#ifndef CK_KEYBOARD_H
#define CK_KEYBOARD_H

#include <stdint.h>

/* Initialize PS/2 keyboard driver. */
void ck_keyboard_init(void);

/* Check if a key is available in the buffer. */
uint8_t ck_keyboard_has_key(void);

/* Read the next key (scancode). Returns 0 if no key available. */
uint8_t ck_keyboard_read_key(void);

/* Read the next ASCII character. Returns 0 if no key or not printable. */
char ck_keyboard_read_char(void);

/* Check if shift is currently pressed. */
uint8_t ck_keyboard_is_shift_pressed(void);

#endif
