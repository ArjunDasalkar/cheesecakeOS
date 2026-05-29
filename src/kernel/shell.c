#include "shell.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../memory/pmem.h"
#include "../memory/heap.h"
#include "scheduler.h"
#include "kernel_tasks.h"
#include "visualizer.h"

/* VGA text buffer */
#define VGA_BUFFER ((unsigned short *)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

/* Colors */
#define COLOR_WHITE     0x0F
#define COLOR_GREEN     0x0A
#define COLOR_YELLOW    0x0E
#define COLOR_RED       0x0C

/*
 * Simple string utilities (no standard library).
 */
static int ck_strlen(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

static int ck_strcmp(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return *a - *b;
}

/* Extract first word from string (command name) */
static void ck_get_first_word(const char *s, char *word, int max_len) {
    int i = 0;
    while (i < max_len - 1 && s[i] && s[i] != ' ') {
        word[i] = s[i];
        i++;
    }
    word[i] = '\0';
}

/* Extract arguments (everything after first word) */
static void ck_get_args(const char *s, char *args, int max_len) {
    int i = 0;
    /* Skip first word */
    while (s[i] && s[i] != ' ') i++;
    /* Skip spaces */
    while (s[i] && s[i] == ' ') i++;
    /* Copy rest */
    int j = 0;
    while (j < max_len - 1 && s[i]) {
        args[j] = s[i];
        i++;
        j++;
    }
    args[j] = '\0';
}

/*
 * Command history (stores last 10 commands)
 */
#define CK_HISTORY_SIZE 10
#define CK_HISTORY_LINE_SIZE 64

static char ck_history[CK_HISTORY_SIZE][CK_HISTORY_LINE_SIZE];
static int ck_history_count = 0;

static void ck_history_add(const char *cmd) {
    if (ck_history_count < CK_HISTORY_SIZE) {
        int i = 0;
        while (i < CK_HISTORY_LINE_SIZE - 1 && cmd[i]) {
            ck_history[ck_history_count][i] = cmd[i];
            i++;
        }
        ck_history[ck_history_count][i] = '\0';
        ck_history_count++;
    } else {
        /* Shift history up and add new one */
        for (int i = 0; i < CK_HISTORY_SIZE - 1; i++) {
            for (int j = 0; j < CK_HISTORY_LINE_SIZE; j++) {
                ck_history[i][j] = ck_history[i + 1][j];
            }
        }
        int i = 0;
        while (i < CK_HISTORY_LINE_SIZE - 1 && cmd[i]) {
            ck_history[CK_HISTORY_SIZE - 1][i] = cmd[i];
            i++;
        }
        ck_history[CK_HISTORY_SIZE - 1][i] = '\0';
    }
}

/*
 * VGA screen buffer management.
 */
static uint16_t ck_cursor_pos = 0;  /* Current cursor position (0-2000) */

static void ck_vga_clear_line(int line) {
    int start = line * VGA_WIDTH;
    int end = start + VGA_WIDTH;
    for (int i = start; i < end; i++) {
        VGA_BUFFER[i] = (COLOR_WHITE << 8) | 0x20;  /* Space */
    }
}

static void ck_vga_write_char(char c, uint8_t color) {
    if (ck_cursor_pos >= 25 * VGA_WIDTH) {
        /* Scroll: shift lines up */
        for (int i = 0; i < 24 * VGA_WIDTH; i++) {
            VGA_BUFFER[i] = VGA_BUFFER[i + VGA_WIDTH];
        }
        ck_vga_clear_line(24);
        ck_cursor_pos = 24 * VGA_WIDTH;
    }
    
    VGA_BUFFER[ck_cursor_pos] = (color << 8) | (unsigned char)c;
    ck_cursor_pos++;
}

static void ck_vga_write_string(const char *s, uint8_t color) {
    while (*s) {
        if (*s == '\n') {
            /* Move to next line */
            int line = ck_cursor_pos / VGA_WIDTH;
            ck_cursor_pos = (line + 1) * VGA_WIDTH;
        } else {
            ck_vga_write_char(*s, color);
        }
        s++;
    }
}

static void ck_vga_newline(void) {
    int line = ck_cursor_pos / VGA_WIDTH;
    ck_cursor_pos = (line + 1) * VGA_WIDTH;
}

static void ck_vga_write_number(uint32_t num, uint8_t color) {
    char buf[16];
    int len = 0;
    uint32_t n = num;
    
    if (n == 0) {
        ck_vga_write_char('0', color);
        return;
    }
    
    while (n > 0) {
        buf[len++] = '0' + (n % 10);
        n /= 10;
    }
    
    for (int i = len - 1; i >= 0; i--) {
        ck_vga_write_char(buf[i], color);
    }
}

static void ck_vga_clear_screen(void) {
    for (int i = 0; i < 25 * VGA_WIDTH; i++) {
        VGA_BUFFER[i] = (COLOR_WHITE << 8) | 0x20;
    }
    ck_cursor_pos = 0;
}

/*
 * Display status line at the top.
 */
static void ck_shell_draw_status(void) {
    uint32_t ticks = ck_timer_get_ticks();
    uint32_t seconds = ticks / 1000;
    uint32_t ms = ticks % 1000;
    
    /* Clear status line */
    ck_vga_clear_line(0);
    
    /* Write status: "Time: XXX.XXXs | CheesecakeOS Shell" */
    uint16_t pos = 0;
    const char *str = "Time: ";
    while (*str) {
        VGA_BUFFER[pos] = (COLOR_YELLOW << 8) | *str++;
        pos++;
    }
    
    /* Write seconds */
    char buf[16];
    int len = 0;
    uint32_t s = seconds;
    if (s == 0) buf[len++] = '0';
    else {
        while (s > 0) {
            buf[len++] = '0' + (s % 10);
            s /= 10;
        }
        for (int i = len - 1; i >= 0; i--) {
            VGA_BUFFER[pos] = (COLOR_YELLOW << 8) | buf[i];
            pos++;
        }
        len = 0;
    }
    
    VGA_BUFFER[pos++] = (COLOR_YELLOW << 8) | '.';
    
    /* Write milliseconds (padded to 3 digits) */
    buf[0] = '0' + ((ms / 100) % 10);
    buf[1] = '0' + ((ms / 10) % 10);
    buf[2] = '0' + (ms % 10);
    for (int i = 0; i < 3; i++) {
        VGA_BUFFER[pos++] = (COLOR_YELLOW << 8) | buf[i];
    }
    
    str = "s";
    while (*str) {
        VGA_BUFFER[pos++] = (COLOR_YELLOW << 8) | *str++;
    }
}

/*
 * Execute a shell command.
 */
static void ck_shell_execute(const char *cmd) {
    /* Track in history */
    if (ck_strlen(cmd) > 0) {
        ck_history_add(cmd);
    }
    
    char cmd_name[64];
    char cmd_args[128];
    ck_get_first_word(cmd, cmd_name, sizeof(cmd_name));
    ck_get_args(cmd, cmd_args, sizeof(cmd_args));
    
    if (ck_strcmp(cmd_name, "help") == 0) {
        ck_vga_write_string("That's grated of you to ask! Here are my flavors:\n", COLOR_GREEN);
        ck_vga_write_string("  help    - the crust of the matter\n", COLOR_WHITE);
        ck_vga_write_string("  uptime  - how long kernel has been baking\n", COLOR_WHITE);
        ck_vga_write_string("  time    - this kernel is on a ROLL (no time to waste)\n", COLOR_WHITE);
        ck_vga_write_string("  echo    - repeat what I tell you\n", COLOR_WHITE);
        ck_vga_write_string("  info    - kernel details (what's in the mix)\n", COLOR_WHITE);
        ck_vga_write_string("  ps      - process list (the kitchen crew)\n", COLOR_WHITE);
        ck_vga_write_string("  memory  - see the cream filling (memory stats)\n", COLOR_WHITE);
        ck_vga_write_string("  tasks   - detailed task counters\n", COLOR_WHITE);
        ck_vga_write_string("  taskmon - live scheduler monitor (watch tasks run!)\n", COLOR_WHITE);
        ck_vga_write_string("  history - show command history\n", COLOR_WHITE);
        ck_vga_write_string("  clear   - clean slate? Un-beet-able!\n", COLOR_WHITE);
        ck_vga_write_string("  reboot  - let's make a fresh bake\n", COLOR_WHITE);
    } else if (ck_strcmp(cmd_name, "uptime") == 0) {
        uint32_t ticks = ck_timer_get_ticks();
        uint32_t seconds = ticks / 1000;
        uint32_t millis = ticks % 1000;
        ck_vga_write_string("System uptime: ", COLOR_GREEN);
        ck_vga_write_number(seconds, COLOR_WHITE);
        ck_vga_write_string(".", COLOR_WHITE);
        ck_vga_write_number(millis / 100, COLOR_WHITE);
        ck_vga_write_string(" seconds (baked to perfection!)\n", COLOR_GREEN);
    } else if (ck_strcmp(cmd_name, "echo") == 0) {
        if (ck_strlen(cmd_args) > 0) {
            ck_vga_write_string(cmd_args, COLOR_YELLOW);
            ck_vga_write_string("\n", COLOR_YELLOW);
        } else {
            ck_vga_write_string("(echo what?)\n", COLOR_YELLOW);
        }
    } else if (ck_strcmp(cmd_name, "info") == 0) {
        ck_vga_write_string("CheesecakeOS v1.0 - Multitasking x86 Kernel\n", COLOR_GREEN);
        ck_vga_write_string("Architecture: 32-bit x86 (i686) Protected Mode\n", COLOR_WHITE);
        ck_vga_write_string("Bootloader: GRUB Multiboot2\n", COLOR_WHITE);
        
        uint32_t total_mem = ck_pmem_get_total_memory();
        uint32_t used_pages = ck_pmem_get_used_pages();
        ck_vga_write_string("Memory: ", COLOR_WHITE);
        ck_vga_write_number(total_mem / 1048576, COLOR_WHITE);
        ck_vga_write_string(" MB | ", COLOR_WHITE);
        ck_vga_write_number(used_pages, COLOR_WHITE);
        ck_vga_write_string(" pages used\n", COLOR_WHITE);
        
        uint32_t heap_used = ck_heap_get_total_allocated() - ck_heap_get_total_freed();
        ck_vga_write_string("Heap: ", COLOR_WHITE);
        ck_vga_write_number(heap_used / 1024, COLOR_WHITE);
        ck_vga_write_string(" KB allocated\n", COLOR_WHITE);
        
        ck_vga_write_string("Interrupts: 32 exceptions + 16 IRQs (PIC remapped)\n", COLOR_WHITE);
        ck_vga_write_string("Timer: 1 kHz PIT (Intel 8254)\n", COLOR_WHITE);
        ck_vga_write_string("Input: PS/2 keyboard\n", COLOR_WHITE);
        
        uint32_t num_tasks = ck_scheduler_task_count();
        ck_vga_write_string("Tasks: ", COLOR_WHITE);
        ck_vga_write_number(num_tasks, COLOR_WHITE);
        ck_vga_write_string(" running | ", COLOR_WHITE);
        ck_vga_write_number(ck_scheduler_get_context_switches(), COLOR_WHITE);
        ck_vga_write_string(" context switches\n", COLOR_WHITE);
    } else if (ck_strcmp(cmd_name, "ps") == 0) {
        ck_vga_write_string("PID  STATE    COUNTER\n", COLOR_GREEN);
        ck_vga_write_string("---  --------  -----------\n", COLOR_WHITE);
        
        uint32_t num_tasks = ck_scheduler_task_count();
        for (uint32_t i = 0; i < num_tasks; i++) {
            struct ck_task *task = ck_scheduler_get_task(i);
            if (task) {
                ck_vga_write_number(i, COLOR_YELLOW);
                ck_vga_write_string("    READY     ", COLOR_WHITE);
                ck_vga_write_number(ck_task_counter[i], COLOR_YELLOW);
                ck_vga_write_string("\n", COLOR_WHITE);
            }
        }
    } else if (ck_strcmp(cmd_name, "history") == 0) {
        ck_vga_write_string("Command History:\n", COLOR_GREEN);
        for (int i = 0; i < ck_history_count; i++) {
            ck_vga_write_number(i + 1, COLOR_YELLOW);
            ck_vga_write_string(": ", COLOR_WHITE);
            ck_vga_write_string(ck_history[i], COLOR_WHITE);
            ck_vga_write_string("\n", COLOR_WHITE);
        }
        if (ck_history_count == 0) {
            ck_vga_write_string("(no commands yet)\n", COLOR_YELLOW);
        }
    } else if (ck_strcmp(cmd_name, "time") == 0) {
        uint32_t ticks = ck_timer_get_ticks();
        ck_vga_write_string("Baking time: ", COLOR_GREEN);
        ck_vga_write_number(ticks / 1000, COLOR_WHITE);
        ck_vga_write_string(".", COLOR_WHITE);
        ck_vga_write_number(ticks % 1000, COLOR_WHITE);
        ck_vga_write_string(" seconds (looking gouda!)\n", COLOR_WHITE);
    } else if (ck_strcmp(cmd_name, "memory") == 0) {
        ck_vga_write_string("Memory Status - A Creamy Filling:\n", COLOR_GREEN);
        
        uint32_t total_mem = ck_pmem_get_total_memory();
        uint32_t used_pages = ck_pmem_get_used_pages();
        uint32_t free_pages = ck_pmem_get_free_pages();
        
        ck_vga_write_string("Total memory: ", COLOR_WHITE);
        ck_vga_write_number(total_mem / 1048576, COLOR_WHITE);
        ck_vga_write_string(" MB (", COLOR_WHITE);
        ck_vga_write_number(total_mem / 4096, COLOR_WHITE);
        ck_vga_write_string(" pages)\n", COLOR_WHITE);
        
        ck_vga_write_string("Used pages: ", COLOR_WHITE);
        ck_vga_write_number(used_pages, COLOR_WHITE);
        ck_vga_write_string(" | Free pages: ", COLOR_WHITE);
        ck_vga_write_number(free_pages, COLOR_WHITE);
        ck_vga_write_string("\n", COLOR_WHITE);
        
        uint32_t heap_alloc = ck_heap_get_total_allocated();
        uint32_t heap_freed = ck_heap_get_total_freed();
        
        ck_vga_write_string("Heap allocated: ", COLOR_WHITE);
        ck_vga_write_number(heap_alloc, COLOR_WHITE);
        ck_vga_write_string(" bytes | Freed: ", COLOR_WHITE);
        ck_vga_write_number(heap_freed, COLOR_WHITE);
        ck_vga_write_string(" bytes\n", COLOR_WHITE);
    } else if (ck_strcmp(cmd_name, "tasks") == 0) {
        ck_vga_write_string("Running Kernel Tasks - The Kitchen Crew:\n", COLOR_GREEN);
        
        uint32_t num_tasks = ck_scheduler_task_count();
        ck_vga_write_string("Total tasks: ", COLOR_WHITE);
        ck_vga_write_number(num_tasks, COLOR_WHITE);
        ck_vga_write_string("\n\n", COLOR_WHITE);
        
        for (uint32_t i = 0; i < num_tasks; i++) {
            struct ck_task *task = ck_scheduler_get_task(i);
            if (task) {
                ck_vga_write_string("Task #", COLOR_YELLOW);
                ck_vga_write_number(i, COLOR_YELLOW);
                ck_vga_write_string(": Counter = ", COLOR_WHITE);
                ck_vga_write_number(ck_task_counter[i], COLOR_WHITE);
                ck_vga_write_string(" (runs: ", COLOR_WHITE);
                ck_vga_write_number(task->times_run, COLOR_WHITE);
                ck_vga_write_string(")\n", COLOR_WHITE);
            }
        }
    } else if (ck_strcmp(cmd_name, "taskmon") == 0) {
        ck_vga_write_string("Entering live task monitor... (press 'q' to exit)\n", COLOR_YELLOW);
        ck_visualizer_run_monitor();
        ck_shell_draw_status();
    } else if (ck_strcmp(cmd_name, "clear") == 0) {
        ck_vga_clear_screen();
        ck_cursor_pos = VGA_WIDTH;  /* Leave status line alone */
        ck_vga_write_string("Fresh and crumbly, just like a new kernel!\n", COLOR_GREEN);
    } else if (ck_strcmp(cmd_name, "reboot") == 0) {
        ck_vga_write_string("Time to serve this kernel... it's reached peak temp!\n", COLOR_RED);
        __asm__("cli");
        __asm__("hlt");
    } else if (ck_strlen(cmd_name) == 0) {
        /* Empty command, do nothing */
    } else {
        ck_vga_write_string("🍪 That command is crumbly... try 'help' for the menu.\n", COLOR_RED);
    }
}

/*
 * Main shell loop.
 */
void ck_shell_run(void) {
    char input_buffer[256];
    int input_len = 0;
    
    ck_vga_clear_screen();
    ck_shell_draw_status();
    
    /* Start at line 2 (line 0 is status, line 1 is blank) */
    ck_cursor_pos = 2 * VGA_WIDTH;
    
    ck_vga_write_string("\n", COLOR_GREEN);
    ck_vga_write_string("  ╔════════════════════════════════════════════╗\n", COLOR_GREEN);
    ck_vga_write_string("  ║  CheesecakeOS - Multitasking Kernel v1.0   ║\n", COLOR_GREEN);
    ck_vga_write_string("  ║     Ready to serve! (May 2026)             ║\n", COLOR_GREEN);
    ck_vga_write_string("  ╚════════════════════════════════════════════╝\n", COLOR_GREEN);
    ck_vga_write_string("\n", COLOR_WHITE);
    ck_vga_write_string("Type 'help' for commands | Try 'taskmon' for live demo\n", COLOR_YELLOW);
    ck_vga_write_string("\n", COLOR_WHITE);
    ck_vga_write_string("> ", COLOR_YELLOW);
    
    while (1) {
        /* Update status every iteration (shows time ticking) */
        ck_shell_draw_status();
        
        /* Run kernel tasks cooperatively (before checking for input) */
        ck_scheduler_run_tasks();
        
        /* Check for keyboard input (non-blocking via buffer) */
        char c = ck_keyboard_read_char();
        if (c == 0) {
            /* No key pressed, continue to next loop iteration */
            continue;
        }
        
        if (c == '\n') {
            /* Execute command */
            input_buffer[input_len] = '\0';
            ck_vga_newline();
            ck_shell_execute(input_buffer);
            input_len = 0;
            ck_vga_write_string("> ", COLOR_YELLOW);
        } else if (c == '\b') {
            /* Backspace */
            if (input_len > 0) {
                input_len--;
                ck_cursor_pos--;
                VGA_BUFFER[ck_cursor_pos] = (COLOR_WHITE << 8) | 0x20;
            }
        } else if (c >= 32 && c < 127) {
            /* Printable character */
            if (input_len < (int)sizeof(input_buffer) - 1) {
                input_buffer[input_len++] = c;
                ck_vga_write_char(c, COLOR_WHITE);
            }
        }
    }
}
