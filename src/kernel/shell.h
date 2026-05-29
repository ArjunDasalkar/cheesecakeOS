#ifndef CK_SHELL_H
#define CK_SHELL_H

#include <stdint.h>

/*
 * Initialize and run the shell.
 * Blocks forever reading keyboard input and executing commands.
 */
void ck_shell_run(void);

#endif
