/*
 * visualizer.h - Task scheduler visualization display
 * 
 * Provides a live monitor of kernel task execution.
 * Displays task counters, activity bars, and scheduler statistics.
 */

#ifndef CK_VISUALIZER_H
#define CK_VISUALIZER_H

#include <stdint.h>

/*
 * Initialize the visualizer.
 * Sets up display buffers and initial state.
 */
void ck_visualizer_init(void);

/*
 * Enter monitor mode: displays live scheduler stats.
 * Blocks until user presses 'q' to exit.
 * Returns 0 on success.
 */
int ck_visualizer_run_monitor(void);

#endif
