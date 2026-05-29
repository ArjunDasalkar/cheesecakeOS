/*
 * kernel_tasks.c - Demo kernel tasks for multitasking
 * 
 * These are simple kernel tasks that demonstrate the multitasking scheduler.
 * Each task is a simple loop that periodically yields.
 */

#include "scheduler.h"
#include <stdint.h>

/* Task-local counters for demonstration */
volatile uint32_t ck_task_counter[8] = {0};

/*
 * Demo task 1: Increments counter and yields frequently.
 */
void ck_task_demo_1(void) {
    while (1) {
        ck_task_counter[0]++;
        
        /* Yield frequently to allow other tasks to run */
        if (ck_task_counter[0] % 1000 == 0) {
            ck_task_yield();
        }
    }
}

/*
 * Demo task 2: Similar to task 1.
 */
void ck_task_demo_2(void) {
    while (1) {
        ck_task_counter[1]++;
        
        if (ck_task_counter[1] % 1500 == 0) {
            ck_task_yield();
        }
    }
}

/*
 * Demo task 3: Similar to task 1.
 */
void ck_task_demo_3(void) {
    while (1) {
        ck_task_counter[2]++;
        
        if (ck_task_counter[2] % 1200 == 0) {
            ck_task_yield();
        }
    }
}
