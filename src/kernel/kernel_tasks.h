/*
 * kernel_tasks.h - Demo kernel tasks
 */

#ifndef CK_KERNEL_TASKS_H
#define CK_KERNEL_TASKS_H

#include <stdint.h>

/* Task counters (for demonstration) */
extern volatile uint32_t ck_task_counter[8];

/* Mini game dimensions for task monitor */
#define CK_GAME_WIDTH 20
#define CK_GAME_HEIGHT 8
#define CK_GAME_PADDLE_WIDTH 6

struct ck_game_state {
	int ball_x;
	int ball_y;
	int ball_dx;
	int ball_dy;
	int paddle_x;
	int frame;
};

void ck_game_init(void);
void ck_game_on_key(char c);
void ck_game_get_state(struct ck_game_state *out);

/* Demo kernel tasks */
void ck_task_demo_1(void);
void ck_task_demo_2(void);
void ck_task_demo_3(void);

#endif
