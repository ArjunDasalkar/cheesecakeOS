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

/* VGA text buffer + screen size */
#define VGA_BUFFER ((unsigned short *)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMSIZE (VGA_WIDTH * VGA_HEIGHT)

/* White on black */
#define VGA_COLOR 0x0F

/* Fill screen with spaces */
void ck_vga_clear_screen(void) {
    int i;
    for (i = 0; i < VGA_MEMSIZE; i++) {
        VGA_BUFFER[i] = (VGA_COLOR << 8) | 0x20;
    }
}

/* Print a string from top-left */
void ck_vga_write_string(const char *str) {
    int pos = 0;
    while (str[pos] != '\0' && pos < VGA_MEMSIZE) {
        VGA_BUFFER[pos] = (VGA_COLOR << 8) | str[pos];
        pos++;
    }
}

/* Entry from boot.asm */
void ck_main(void) {
    /* Ensure interrupts are disabled during early init. */
    __asm__("cli");

    ck_vga_clear_screen();
    ck_vga_write_string("Initializing CheesecakeOS...");

    ck_idt_init();
    ck_exceptions_init();
    ck_pic_init();
    ck_irq_init();
    ck_timer_init();
    ck_keyboard_init();
    
    /* Initialize memory management */
    ck_pmem_init(NULL);   /* TODO: Pass GRUB multiboot info */
    ck_paging_init();
    ck_heap_init();

    /* Enable interrupts */
    __asm__("sti");

    /* Start the shell */
    ck_shell_run();
}
