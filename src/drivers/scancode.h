#ifndef CK_SCANCODE_H
#define CK_SCANCODE_H

#include <stdint.h>

/*
 * PS/2 scancode to ASCII conversion tables (US layout).
 * Index = scancode, value = ASCII character (0 if not printable).
 * Two tables: unshifted (lowercase) and shifted (uppercase/symbols).
 */
extern const uint8_t ck_scancode_table[256];           /* Unshifted */
extern const uint8_t ck_scancode_table_shifted[256];   /* Shifted */

/*
 * Convert a PS/2 scancode to ASCII.
 * shift_pressed: 1 if shift is held, 0 otherwise.
 * Returns 0 if not printable (e.g., modifier keys, extended codes).
 */
static inline uint8_t ck_scancode_to_ascii(uint8_t scancode, uint8_t shift_pressed) {
    if (shift_pressed) {
        return ck_scancode_table_shifted[scancode];
    } else {
        return ck_scancode_table[scancode];
    }
}

#endif
