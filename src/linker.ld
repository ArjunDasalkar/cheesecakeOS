/* linker.ld — arrange boot.asm + kernel code into bootable binary */

/* Entry point: ck_start from boot.asm */
ENTRY(ck_start)

/* Configure the output format and machine architecture */
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")

/* Load at 1MB (0x100000) — standard bootloader address */
SECTIONS
{

    . = 0x100000;

    /* Multiboot2 header — must be first 32KB */
    .multiboot2 :
    {
        KEEP(*(.multiboot2))
    }

    /* Code section (.text) — boot.asm + kernel */
    .text :
    {
        KEEP(*(.text))
        *(.text*)
    }

    /* Read-only data (.rodata) — string literals, etc */
    .rodata :
    {
        *(.rodata)
        *(.rodata*)
    }

    /* Initialized data (.data) — global vars with values */
    .data :
    {
        *(.data)
        *(.data*)
    }

    /* Uninitialized data (.bss) — stack (16KB from boot.asm) */
    .bss :
    {
        KEEP(*(.bss))
        *(.bss*)
        *(COMMON)
    }

    /* Remove debug symbols & compiler cruft */
    /DISCARD/ :
    {
        *(.note.GNU-stack)
        *(.gnu_debuglink)
        *(.gnu.lto_*)
    }
}
