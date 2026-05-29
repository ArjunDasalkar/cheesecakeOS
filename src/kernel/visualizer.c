/*
 * visualizer.c - Task scheduler visualization display
 * 
 * Live monitor showing:
 * - Task counter values (per-task activity)
 * - Progress bars (relative task utilization)
 * - Scheduler statistics (total switches, heap usage)
 */

#include "visualizer.h"
#include "scheduler.h"
#include "kernel_tasks.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include <stdint.h>
#include <string.h>

/*
 * VGA display buffer and utilities.
 */
#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR_WHITE 0x0F
#define VGA_COLOR_CYAN 0x0B
#define VGA_COLOR_GREEN 0x0A
#define VGA_COLOR_YELLOW 0x0E

#define GAME_TOP 3
#define GAME_LEFT 52

static const char *ck_task_name(uint32_t id) {
    switch (id) {
        case 0: return "Physics";
        case 1: return "Input";
        case 2: return "Render";
        default: return "Worker";
    }
}

/* VGA cursor control */
#define VGA_CURSOR_CMD 0x3D4
#define VGA_CURSOR_DATA 0x3D5

static inline void vga_outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void vga_cursor_disable(void) {
    vga_outb(VGA_CURSOR_CMD, 0x0A);
    vga_outb(VGA_CURSOR_DATA, 0x20);
}

static void vga_cursor_enable(void) {
    vga_outb(VGA_CURSOR_CMD, 0x0A);
    vga_outb(VGA_CURSOR_DATA, 0x0F);
    vga_outb(VGA_CURSOR_CMD, 0x0B);
    vga_outb(VGA_CURSOR_DATA, 0x0F);
}

/*
 * Write a character to VGA at (row, col).
 */
static void vga_write_char(uint16_t row, uint16_t col, char c, uint8_t color) {
    if (row >= VGA_HEIGHT || col >= VGA_WIDTH) {
        return;
    }
    uint16_t *vga = (uint16_t *)VGA_ADDRESS;
    uint16_t offset = row * VGA_WIDTH + col;
    vga[offset] = ((color << 8) | c);
}

/*
 * Write a string to VGA at (row, col).
 */
static void vga_write_string(uint16_t row, uint16_t col, const char *str, uint8_t color) {
    uint16_t c = col;
    while (*str && c < VGA_WIDTH) {
        vga_write_char(row, c, *str, color);
        c++;
        str++;
    }
}

/*
 * Write a uint32_t as decimal to VGA.
 */
static void vga_write_uint32(uint16_t row, uint16_t col, uint32_t num, uint8_t color) {
    char buffer[12];  /* Max 10 digits + null + sign */
    int pos = 0;
    
    if (num == 0) {
        buffer[pos++] = '0';
    } else {
        uint32_t div = 1000000000;
        int started = 0;
        while (div > 0) {
            uint32_t digit = (num / div) % 10;
            if (digit || started) {
                buffer[pos++] = '0' + digit;
                started = 1;
            }
            div /= 10;
        }
    }
    buffer[pos] = '\0';
    vga_write_string(row, col, buffer, color);
}

/*
 * Fill a row with spaces (clear).
 */
static void vga_clear_row(uint16_t row, uint8_t color) {
    for (uint16_t col = 0; col < VGA_WIDTH; col++) {
        vga_write_char(row, col, ' ', color);
    }
}

static void draw_game_frame(void) {
    struct ck_game_state state;
    ck_game_get_state(&state);

    uint16_t top = GAME_TOP;
    uint16_t left = GAME_LEFT;
    uint16_t width = CK_GAME_WIDTH;
    uint16_t height = CK_GAME_HEIGHT;

    for (uint16_t r = 0; r < height + 2; r++) {
        for (uint16_t c = 0; c < width + 2; c++) {
            char ch = ' ';
            if (r == 0 || r == height + 1) {
                ch = (c == 0 || c == width + 1) ? '+' : '-';
            } else if (c == 0 || c == width + 1) {
                ch = '|';
            }
            vga_write_char(top + r, left + c, ch, VGA_COLOR_WHITE);
        }
    }

    vga_write_char(top + 1 + state.ball_y, left + 1 + state.ball_x, 'o', VGA_COLOR_YELLOW);

    uint16_t paddle_row = top + height;
    for (int i = 0; i < CK_GAME_PADDLE_WIDTH; i++) {
        vga_write_char(paddle_row, left + 1 + state.paddle_x + i, '=', VGA_COLOR_GREEN);
    }
}

/*
 * Draw a progress bar: [####........] for a given percentage.
 * Bar is 20 chars wide, printed at (row, col).
 */
static void draw_progress_bar(uint16_t row, uint16_t col, uint32_t max_val, uint32_t current_val, uint8_t color) {
    char bar[23];
    bar[0] = '[';
    
    uint32_t filled;
    if (max_val == 0) {
        filled = 0;
    } else {
        filled = (current_val * 20) / max_val;
        if (filled > 20) filled = 20;
    }
    
    for (uint32_t i = 0; i < 20; i++) {
        if (i < filled) {
            bar[1 + i] = '#';
        } else {
            bar[1 + i] = '.';
        }
    }
    bar[21] = ']';
    bar[22] = '\0';
    
    vga_write_string(row, col, bar, color);
}

/*
 * Initialize visualizer.
 */
void ck_visualizer_init(void) {
    /* Nothing to initialize currently */
}

/*
 * Run the live task monitor.
 * Displays task stats in a loop until user presses 'q'.
 */
int ck_visualizer_run_monitor(void) {
    static uint32_t prev_counters[8] = {0};
    uint32_t last_draw = 0;
    const uint32_t frame_ms = 50;
    int exit_requested = 0;
    ck_game_init();

    vga_cursor_disable();

    /* Clear screen */
    for (uint16_t row = 0; row < VGA_HEIGHT; row++) {
        vga_clear_row(row, VGA_COLOR_WHITE);
    }
    
    /* Main display loop */
    while (1) {
        /* Handle input (drain buffer) */
        while (1) {
            char c = ck_keyboard_read_char();
            if (c == 0) {
                break;
            }
            if (c == 'q' || c == 'Q') {
                exit_requested = 1;
            } else if (c == 'a' || c == 'A' || c == 'd' || c == 'D') {
                ck_game_on_key(c);
            }
        }
        if (exit_requested) {
            break;
        }

        /* Throttle rendering to keep motion visible */
        uint32_t now = ck_timer_get_ticks();
        if ((now - last_draw) < frame_ms) {
            for (volatile uint32_t i = 0; i < 20000; i++) {
                asm("nop");
            }
            continue;
        }
        last_draw = now;

        /* Advance tasks so counters change while monitoring */
        ck_scheduler_run_tasks();

        /* Draw header */
        vga_clear_row(0, VGA_COLOR_CYAN);
        vga_write_string(0, 1, "CheesecakeOS Task Scheduler Monitor", VGA_COLOR_CYAN);
        
        vga_clear_row(1, VGA_COLOR_WHITE);
        vga_write_string(1, 1, "===============================================", VGA_COLOR_WHITE);

        vga_clear_row(2, VGA_COLOR_WHITE);
        vga_write_string(2, 1, "A/D move paddle | Q quit", VGA_COLOR_YELLOW);

        draw_game_frame();
        
        /* Find total delta for normalization */
        uint32_t total_delta = 0;
        uint32_t deltas[8] = {0};
        for (uint32_t i = 0; i < ck_scheduler_task_count(); i++) {
            deltas[i] = ck_task_counter[i] - prev_counters[i];
            prev_counters[i] = ck_task_counter[i];
            total_delta += deltas[i];
        }
        if (total_delta == 0) {
            total_delta = 1;
        }
        
        /* Draw task rows */
        for (uint32_t i = 0; i < ck_scheduler_task_count(); i++) {
            uint16_t row = 12 + i;
            if (row >= 20) break;  /* Leave room for footer */
            
            struct ck_task *task = ck_scheduler_get_task(i);
            if (!task) continue;
            
            vga_clear_row(row, VGA_COLOR_WHITE);
            
            /* Task label */
            vga_write_string(row, 1, "Task ", VGA_COLOR_GREEN);
            vga_write_uint32(row, 6, i, VGA_COLOR_GREEN);
            vga_write_string(row, 8, " ", VGA_COLOR_WHITE);
            vga_write_string(row, 9, ck_task_name(i), VGA_COLOR_CYAN);
            
            /* Progress bar */
            draw_progress_bar(row, 22, total_delta, deltas[i], VGA_COLOR_YELLOW);
            
            /* Counter value */
            vga_write_string(row, 44, "CPU: ", VGA_COLOR_WHITE);
            uint32_t pct = (deltas[i] * 100) / total_delta;
            vga_write_uint32(row, 49, pct, VGA_COLOR_YELLOW);
            vga_write_string(row, 52, "% Cnt: ", VGA_COLOR_WHITE);
            vga_write_uint32(row, 59, ck_task_counter[i], VGA_COLOR_YELLOW);
        }
        
        /* Summary stats line */
        uint16_t stats_row = 20;
        vga_clear_row(stats_row, VGA_COLOR_WHITE);
        
        uint32_t active = ck_scheduler_task_count();
        uint32_t switches = ck_scheduler_get_context_switches();
        
        vga_write_string(stats_row, 1, "Active: ", VGA_COLOR_WHITE);
        vga_write_uint32(stats_row, 9, active, VGA_COLOR_GREEN);
        
        vga_write_string(stats_row, 16, "| Switches: ", VGA_COLOR_WHITE);
        vga_write_uint32(stats_row, 28, switches, VGA_COLOR_GREEN);
        
        /* Footer */
        uint16_t footer = 23;
        vga_clear_row(footer, VGA_COLOR_WHITE);
        vga_write_string(footer, 1, "[q] Quit monitor", VGA_COLOR_YELLOW);
        
        /* Small delay to avoid busy loop */
        for (volatile uint32_t i = 0; i < 100000; i++) {
            asm("nop");
        }
    }

    vga_cursor_enable();
    
    return 0;
}
