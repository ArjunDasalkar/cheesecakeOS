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

/*
 * Draw a progress bar: [████░░░░░░░░░░] for a given percentage.
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
            bar[1 + i] = 0xFE;  /* Full block character */
        } else {
            bar[1 + i] = 0xF0;  /* Light shade character */
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
    /* Clear screen */
    for (uint16_t row = 0; row < VGA_HEIGHT; row++) {
        vga_clear_row(row, VGA_COLOR_WHITE);
    }
    
    /* Main display loop */
    while (1) {
        /* Draw header */
        vga_clear_row(0, VGA_COLOR_CYAN);
        vga_write_string(0, 1, "CheesecakeOS Task Scheduler Monitor", VGA_COLOR_CYAN);
        
        vga_clear_row(1, VGA_COLOR_WHITE);
        vga_write_string(1, 1, "===============================================", VGA_COLOR_WHITE);
        
        /* Find max task counter for normalization */
        uint32_t max_counter = 1;
        for (uint32_t i = 0; i < ck_scheduler_task_count(); i++) {
            uint32_t cnt = ck_task_counter[i];
            if (cnt > max_counter) {
                max_counter = cnt;
            }
        }
        
        /* Draw task rows */
        for (uint32_t i = 0; i < ck_scheduler_task_count(); i++) {
            uint16_t row = 3 + i;
            if (row >= 20) break;  /* Leave room for footer */
            
            struct ck_task *task = ck_scheduler_get_task(i);
            if (!task) continue;
            
            vga_clear_row(row, VGA_COLOR_WHITE);
            
            /* Task label */
            vga_write_string(row, 1, "Task ", VGA_COLOR_GREEN);
            vga_write_uint32(row, 6, i, VGA_COLOR_GREEN);
            vga_write_string(row, 8, " ", VGA_COLOR_WHITE);
            
            /* Progress bar */
            draw_progress_bar(row, 9, max_counter, ck_task_counter[i], VGA_COLOR_YELLOW);
            
            /* Counter value */
            vga_write_string(row, 31, "Counter: ", VGA_COLOR_WHITE);
            vga_write_uint32(row, 40, ck_task_counter[i], VGA_COLOR_YELLOW);
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
        
        /* Check for input */
        char c = ck_keyboard_read_char();
        if (c == 'q' || c == 'Q') {
            break;
        }
        
        /* Small delay to avoid busy loop */
        for (volatile uint32_t i = 0; i < 1000000; i++) {
            asm("nop");
        }
    }
    
    return 0;
}
