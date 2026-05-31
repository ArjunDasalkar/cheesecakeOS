/* ck_kernel.c - basic VGA hello */

#include "../interrupts/idt.h"
#include "../interrupts/exceptions.h"
#include "../interrupts/pic.h"
#include "../interrupts/irq.h"
#include "../drivers/timer.h"
#include "../drivers/keyboard.h"
#include "../memory/pmem.h"
#include "../memory/paging.h"
#include "../memory/heap.h"
#include "shell.h"
#include "scheduler.h"
#include "kernel_tasks.h"

/* VGA text buffer + screen size */
#define VGA_BUFFER ((unsigned short *)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMSIZE (VGA_WIDTH * VGA_HEIGHT)

/* White on black */
#define VGA_COLOR 0x0F

static uint16_t ck_boot_cursor = 0;

static void ck_boot_delay(void) {
    for (volatile uint32_t i = 0; i < 50000000; i++) {
        __asm__ volatile ("nop");
    }
}

/* Fill screen with spaces */
void ck_vga_clear_screen(void) {
    int i;
    for (i = 0; i < VGA_MEMSIZE; i++) {
        VGA_BUFFER[i] = (VGA_COLOR << 8) | 0x20;
    }
    ck_boot_cursor = 0;
}

static void ck_vga_write_char(char c) {
    if (ck_boot_cursor >= VGA_MEMSIZE) {
        return;
    }
    VGA_BUFFER[ck_boot_cursor] = (VGA_COLOR << 8) | (unsigned char)c;
    ck_boot_cursor++;
}

/* Print a string from the current cursor position */
void ck_vga_write_string(const char *str) {
    int pos = 0;
    while (str[pos] != '\0') {
        if (str[pos] == '\n') {
            int line = ck_boot_cursor / VGA_WIDTH;
            ck_boot_cursor = (line + 1) * VGA_WIDTH;
        } else {
            ck_vga_write_char(str[pos]);
        }
        pos++;
    }
}

/* Entry from boot.asm */
void ck_main(void) {
    /* Ensure interrupts are disabled during early init. */
    __asm__("cli");

    ck_vga_clear_screen();
    ck_vga_write_string("CheesecakeOS v1.0\n");
    ck_boot_delay();
    ck_vga_write_string("Initializing IDT...\n");
    ck_boot_delay();

    ck_idt_init();
    ck_vga_write_string("Initializing PIC...\n");
    ck_boot_delay();
    ck_exceptions_init();
    ck_pic_init();
    ck_vga_write_string("Initializing IRQs...\n");
    ck_boot_delay();
    ck_irq_init();
    ck_vga_write_string("Initializing Timer...\n");
    ck_boot_delay();
    ck_timer_init();
    ck_vga_write_string("Initializing Keyboard...\n");
    ck_boot_delay();
    ck_keyboard_init();
    
    /* Initialize memory management */
    ck_vga_write_string("Initializing Memory Manager...\n");
    ck_boot_delay();
    ck_pmem_init(NULL);   /* TODO: Pass GRUB multiboot info */
    ck_paging_init();
    ck_heap_init();
    
    /* Initialize scheduler and create demo tasks */
    ck_vga_write_string("Initializing Scheduler...\n");
    ck_boot_delay();
    ck_scheduler_init();
    
    struct ck_task task1, task2, task3;
    ck_task_create(&task1, ck_task_demo_1, 4096);  /* 4KB stack per task */
    ck_task_create(&task2, ck_task_demo_2, 4096);
    ck_task_create(&task3, ck_task_demo_3, 4096);

    ck_vga_write_string("\nSystem Ready.\n");
    ck_boot_delay();

    /* Enable interrupts */
    __asm__("sti");

    /* Start the shell */
    ck_shell_run();
}
