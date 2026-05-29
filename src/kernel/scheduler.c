/*
 * scheduler.c - Task scheduling and context switching implementation
 * 
 * Implements a simple cooperative multitasking scheduler.
 * Context switches occur when a task calls ck_task_yield().
 * 
 * Design:
 * - Each task has its own stack and execution state
 * - Tasks are run as functions that yield control periodically
 * - When a task yields, the scheduler selects the next ready task
 * - Tasks are stored in a simple array and cycled through
 */

#include "scheduler.h"
#include "../memory/heap.h"
#include <string.h>

/*
 * Maximum number of tasks.
 */
#define CK_SCHEDULER_MAX_TASKS 8

/*
 * Task table and state.
 */
static struct ck_task ck_task_table[CK_SCHEDULER_MAX_TASKS];
static uint32_t ck_task_count = 0;
static uint32_t ck_current_task_id = 0;

/*
 * Saved execution context for cooperative multitasking.
 * For cooperative mode, we just need to know which task is running.
 */
struct ck_task_context {
    struct ck_task *task;
    uint32_t call_depth;  /* Track recursion depth for yield calls */
};

static struct ck_task_context ck_current_context = {NULL, 0};

/*
 * Initialize the scheduler.
 */
void ck_scheduler_init(void) {
    memset(ck_task_table, 0, sizeof(ck_task_table));
    ck_task_count = 0;
    ck_current_task_id = 0;
    ck_current_context.task = NULL;
    ck_current_context.call_depth = 0;
}

/*
 * Create a new kernel task.
 */
int32_t ck_task_create(struct ck_task *task, ck_task_entry_t entry, uint32_t stack_size) {
    if (ck_task_count >= CK_SCHEDULER_MAX_TASKS) {
        return -1;  /* Task table full */
    }
    
    if (!task || !entry || stack_size == 0) {
        return -1;  /* Invalid arguments */
    }
    
    /* Allocate stack */
    uint8_t *stack = (uint8_t *)ck_malloc(stack_size);
    if (!stack) {
        return -1;  /* Memory allocation failed */
    }
    
    /* Allocate registers structure */
    struct ck_registers *regs = (struct ck_registers *)ck_malloc(sizeof(struct ck_registers));
    if (!regs) {
        ck_free(stack);
        return -1;
    }
    
    /* Initialize task structure */
    task->id = ck_task_count;
    task->state = CK_TASK_READY;
    task->entry = entry;
    task->regs = regs;
    task->stack = stack;
    task->stack_size = stack_size;
    task->ticks_remaining = 0;  /* Not used in cooperative mode */
    task->times_run = 0;
    task->total_ticks = 0;
    
    /* Add to task table */
    ck_task_table[ck_task_count] = *task;
    uint32_t task_id = ck_task_count;
    ck_task_count++;
    
    return task_id;
}

/*
 * Get the currently running task.
 */
struct ck_task *ck_scheduler_current_task(void) {
    if (ck_current_task_id < ck_task_count) {
        return &ck_task_table[ck_current_task_id];
    }
    return NULL;
}

/*
 * Get the number of active tasks.
 */
uint32_t ck_scheduler_task_count(void) {
    return ck_task_count;
}

/*
 * Get a task by ID.
 */
struct ck_task *ck_scheduler_get_task(uint32_t id) {
    if (id < ck_task_count) {
        return &ck_task_table[id];
    }
    return NULL;
}

/*
 * Run all kernel tasks cooperatively.
 * 
 * This function continuously cycles through tasks, giving each a chance to run
 * until they yield. This is called from the main shell loop.
 */
void ck_scheduler_run_tasks(void) {
    if (ck_task_count == 0) {
        return;
    }
    
    /* Run a single iteration of each task (round-robin) */
    for (uint32_t i = 0; i < ck_task_count; i++) {
        ck_current_task_id = i;
        struct ck_task *task = &ck_task_table[i];
        
        if (task->state == CK_TASK_READY || task->state == CK_TASK_RUNNING) {
            task->state = CK_TASK_RUNNING;
            task->times_run++;
            
            ck_current_context.task = task;
            ck_current_context.call_depth = 0;
            
            /* Call the task function */
            if (task->entry) {
                task->entry();
            }
            
            task->state = CK_TASK_READY;
        }
    }
}

/*
 * Scheduler: Switch to next task (cooperative multitasking).
 * 
 * This is a "no-op" in our simple cooperative implementation.
 * Tasks are cycled in ck_scheduler_run_tasks() instead.
 * This function can be called to yield, but actual switching happens
 * when the Shell calls ck_scheduler_run_tasks().
 */
void ck_schedule(void) {
    /* In our cooperative model, just increment statistics */
    if (ck_current_context.task) {
        ck_current_context.task->total_ticks++;
    }
}

/*
 * Yield control from current task to scheduler.
 * 
 * In the cooperative model, this just increments counters.
 * Actual task switching happens in the Shell's main loop
 * which calls ck_scheduler_run_tasks().
 */
void ck_task_yield(void) {
    ck_schedule();
    /* Return control to the current task function */
}
