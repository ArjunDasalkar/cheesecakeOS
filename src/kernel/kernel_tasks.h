/*
 * kernel_tasks.h - Demo kernel tasks
 */

#ifndef CK_KERNEL_TASKS_H
#define CK_KERNEL_TASKS_H

#include <stdint.h>

/* Task counters (for demonstration) */
extern volatile uint32_t ck_task_counter[8];

/* Demo kernel tasks */
void ck_task_demo_1(void);
void ck_task_demo_2(void);
void ck_task_demo_3(void);

#endif
