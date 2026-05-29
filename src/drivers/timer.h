#ifndef CK_TIMER_H
#define CK_TIMER_H

#include <stdint.h>

/* Initialize the system timer (PIT) to fire at 1 kHz. */
void ck_timer_init(void);

/* Get the current tick count. */
uint32_t ck_timer_get_ticks(void);

#endif
