/*
 * kernel_tasks.c - Demo kernel tasks for multitasking
 * 
 * These are simple kernel tasks that demonstrate the multitasking scheduler.
 * Each task performs a small step and returns so the scheduler can continue.
 */

#include "kernel_tasks.h"
#include "scheduler.h"
#include <stdint.h>

/* Task-local counters for demonstration */
volatile uint32_t ck_task_counter[8] = {0};

static struct ck_game_state ck_game = {0};
static int ck_game_input_dir = 0;

void ck_game_init(void) {
    ck_game.ball_x = 2;
    ck_game.ball_y = 2;
    ck_game.ball_dx = 1;
    ck_game.ball_dy = 1;
    ck_game.paddle_x = (CK_GAME_WIDTH - CK_GAME_PADDLE_WIDTH) / 2;
    ck_game.frame = 0;
    ck_game_input_dir = 0;
}

void ck_game_on_key(char c) {
    if (c == 'a' || c == 'A') {
        ck_game_input_dir = -1;
    } else if (c == 'd' || c == 'D') {
        ck_game_input_dir = 1;
    }
}

void ck_game_get_state(struct ck_game_state *out) {
    if (!out) {
        return;
    }
    *out = ck_game;
}

/*
 * Demo task 1: Increments counter and yields frequently.
 */
void ck_task_demo_1(void) {
    int work = 1;

    ck_game.frame++;

    if ((ck_game.frame % 2) == 0) {
        ck_game.ball_x += ck_game.ball_dx;
        ck_game.ball_y += ck_game.ball_dy;
    }

    if (ck_game.ball_x <= 0 || ck_game.ball_x >= (CK_GAME_WIDTH - 1)) {
        if (ck_game.ball_x <= 0) {
            ck_game.ball_x = 0;
            ck_game.ball_dx = 1;
        } else {
            ck_game.ball_x = CK_GAME_WIDTH - 1;
            ck_game.ball_dx = -1;
        }
        work += 6;
    }

    if (ck_game.ball_y <= 0) {
        ck_game.ball_y = 0;
        ck_game.ball_dy = 1;
        work += 6;
    }

    if (ck_game.ball_y >= (CK_GAME_HEIGHT - 2)) {
        int paddle_end = ck_game.paddle_x + CK_GAME_PADDLE_WIDTH - 1;
        if (ck_game.ball_x >= ck_game.paddle_x && ck_game.ball_x <= paddle_end) {
            ck_game.ball_y = CK_GAME_HEIGHT - 2;
            ck_game.ball_dy = -1;
            work += 10;
        } else {
            ck_game.ball_y = 1;
            ck_game.ball_x = 2;
            ck_game.ball_dx = 1;
            ck_game.ball_dy = 1;
            work += 12;
        }
    }

    ck_task_counter[0] += (uint32_t)work;

    /* Yield occasionally to exercise scheduler stats */
    if (ck_task_counter[0] % 1000 == 0) {
        ck_task_yield();
    }
}

/*
 * Demo task 2: Similar to task 1.
 */
void ck_task_demo_2(void) {
    int work = 1;
    if (ck_game_input_dir != 0) {
        ck_game.paddle_x += ck_game_input_dir;
        ck_game_input_dir = 0;
        work += 5;
    } else {
        int center = (CK_GAME_WIDTH - CK_GAME_PADDLE_WIDTH) / 2;
        if (ck_game.paddle_x < center) {
            ck_game.paddle_x++;
            work += 2;
        } else if (ck_game.paddle_x > center) {
            ck_game.paddle_x--;
            work += 2;
        }
    }

    if (ck_game.paddle_x < 0) {
        ck_game.paddle_x = 0;
    } else if (ck_game.paddle_x > (CK_GAME_WIDTH - CK_GAME_PADDLE_WIDTH)) {
        ck_game.paddle_x = CK_GAME_WIDTH - CK_GAME_PADDLE_WIDTH;
    }

    ck_task_counter[1] += (uint32_t)work;

    if (ck_task_counter[1] % 1500 == 0) {
        ck_task_yield();
    }
}

/*
 * Demo task 3: Similar to task 1.
 */
void ck_task_demo_3(void) {
    int work = 1 + (ck_game.ball_y % 4);
    ck_task_counter[2] += (uint32_t)work;

    if (ck_task_counter[2] % 1200 == 0) {
        ck_task_yield();
    }
}
