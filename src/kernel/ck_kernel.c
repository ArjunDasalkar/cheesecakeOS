/* ck_kernel.c - basic VGA hello */

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
    ck_vga_clear_screen();
    ck_vga_write_string("CheesecakeOS served fresh...");

    /* Stay here */
    while (1) {
        __asm__("hlt");
    }
}
