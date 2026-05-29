/*
 * scheduler.h - Task scheduling and context switching
 * 
 * CheesecakeOS multitasking implementation.
 * Implements cooperative round-robin scheduling.
 */

#ifndef CK_SCHEDULER_H
#define CK_SCHEDULER_H

#include <stdint.h>
#include "../interrupts/exceptions.h"

/*
 * Task states.
 */
typedef enum {
    CK_TASK_READY = 0,
    CK_TASK_RUNNING = 1,
    CK_TASK_BLOCKED = 2,
} ck_task_state_t;

/*
 * Task entry point function signature.
 * Each kernel task is a function that runs in kernel mode.
 */
typedef void (*ck_task_entry_t)(void);

/*
 * Task Control Block (TCB).
 * Represents a single kernel task.
 */
struct ck_task {
    uint32_t id;                        /* Task ID (0 = first task) */
    ck_task_state_t state;              /* Current state */
    ck_task_entry_t entry;              /* Task entry point */
    struct ck_registers *regs;          /* Saved CPU registers */
    uint8_t *stack;                     /* Task stack */
    uint32_t stack_size;                /* Stack size in bytes */
    uint32_t ticks_remaining;           /* Time slice ticks remaining */
    uint32_t times_run;                 /* Statistics: times scheduled */
    uint32_t total_ticks;               /* Statistics: total ticks executed */
};

/*
 * Initialize the scheduler.
 * Sets up the task table.
 */
void ck_scheduler_init(void);

/*
 * Create a new kernel task.
 * 
 * task: Pointer to task structure to initialize
 * entry: Function pointer to task entry point
 * stack_size: Size of stack to allocate (in bytes)
 * 
 * Returns: Task ID on success, -1 on error
 */
int32_t ck_task_create(struct ck_task *task, ck_task_entry_t entry, uint32_t stack_size);

/*
 * Get the currently running task.
 */
struct ck_task *ck_scheduler_current_task(void);

/*
 * Get the number of active tasks.
 */
uint32_t ck_scheduler_task_count(void);

/*
 * Get a task by ID.
 */
struct ck_task *ck_scheduler_get_task(uint32_t id);

/*
 * Get the total number of context switches.
 * Useful for monitoring scheduler activity.
 */
uint32_t ck_scheduler_get_context_switches(void);

/*
 * Run all kernel tasks cooperatively.
 * This function continuously cycles through tasks, giving each a chance to run.
 * Called from the shell's main loop.
 */
void ck_scheduler_run_tasks(void);

/*
 * Yield control from current task.
 * In cooperative mode, this just updates statistics.
 */
void ck_task_yield(void);

#endif
