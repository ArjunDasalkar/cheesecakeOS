# CheesecakeOS build flow:
# asm -> obj, c -> obj, link -> kernel, pack -> iso, run -> qemu

CC := gcc
AS := nasm
LD := ld
GRUB_MKRESCUE := grub-mkrescue
QEMU := qemu-system-i386

# Paths
BOOT_DIR := boot
SRC_DIR := src
BUILD_DIR := build
KERNEL_DIR := $(SRC_DIR)/kernel
INTERRUPTS_DIR := $(SRC_DIR)/interrupts
EXCEPTIONS_DIR := $(SRC_DIR)/interrupts
DRIVERS_DIR := $(SRC_DIR)/drivers
MEMORY_DIR := $(SRC_DIR)/memory

# Freestanding 32-bit kernel flags
CFLAGS := -m32 -ffreestanding -Wall -Wextra -Werror -Os -fno-builtin -fno-stack-protector

# NASM output format
ASFLAGS := -f elf32

# Link with custom layout, no stdlib
LDFLAGS := -m elf_i386 -T $(SRC_DIR)/linker.ld -static -nostdlib

# Inputs/outputs
BOOT_ASM := $(BOOT_DIR)/boot.asm
KERNEL_C := $(KERNEL_DIR)/ck_kernel.c
INTERRUPTS_C := $(INTERRUPTS_DIR)/idt.c
ISR_ASM := $(INTERRUPTS_DIR)/isr.asm
EXCEPTIONS_C := $(EXCEPTIONS_DIR)/exceptions.c
EXCEPTIONS_ASM := $(EXCEPTIONS_DIR)/exceptions.asm
PIC_C := $(INTERRUPTS_DIR)/pic.c
IRQ_C := $(INTERRUPTS_DIR)/irq.c
IRQ_ASM := $(INTERRUPTS_DIR)/irq.asm
TIMER_C := $(DRIVERS_DIR)/timer.c
KEYBOARD_C := $(DRIVERS_DIR)/keyboard.c
SCANCODE_C := $(DRIVERS_DIR)/scancode.c
SHELL_C := $(SRC_DIR)/kernel/shell.c
SCHEDULER_C := $(SRC_DIR)/kernel/scheduler.c
KERNEL_TASKS_C := $(SRC_DIR)/kernel/kernel_tasks.c
VISUALIZER_C := $(SRC_DIR)/kernel/visualizer.c
PMEM_C := $(MEMORY_DIR)/pmem.c
PAGING_C := $(MEMORY_DIR)/paging.c
HEAP_C := $(MEMORY_DIR)/heap.c
LINKER_SCRIPT := $(SRC_DIR)/linker.ld
GRUB_CFG := $(BOOT_DIR)/grub/grub.cfg

BOOT_OBJ := $(BUILD_DIR)/boot.o
KERNEL_OBJ := $(BUILD_DIR)/ck_kernel.o
INTERRUPTS_OBJ := $(BUILD_DIR)/idt.o
ISR_OBJ := $(BUILD_DIR)/isr.o
EXCEPTIONS_OBJ := $(BUILD_DIR)/exceptions.o
EXCEPTIONS_ASM_OBJ := $(BUILD_DIR)/exceptions_asm.o
PIC_OBJ := $(BUILD_DIR)/pic.o
IRQ_OBJ := $(BUILD_DIR)/irq.o
IRQ_ASM_OBJ := $(BUILD_DIR)/irq_asm.o
TIMER_OBJ := $(BUILD_DIR)/timer.o
KEYBOARD_OBJ := $(BUILD_DIR)/keyboard.o
SCANCODE_OBJ := $(BUILD_DIR)/scancode.o
SHELL_OBJ := $(BUILD_DIR)/shell.o
SCHEDULER_OBJ := $(BUILD_DIR)/scheduler.o
KERNEL_TASKS_OBJ := $(BUILD_DIR)/kernel_tasks.o
VISUALIZER_OBJ := $(BUILD_DIR)/visualizer.o
PMEM_OBJ := $(BUILD_DIR)/pmem.o
PAGING_OBJ := $(BUILD_DIR)/paging.o
HEAP_OBJ := $(BUILD_DIR)/heap.o
KERNEL_BIN := $(BUILD_DIR)/cheesecake.bin
ISO_IMAGE := $(BUILD_DIR)/cheesecake.iso
ISO_ROOT := $(BUILD_DIR)/isodir

.PHONY: all iso run clean

all: $(KERNEL_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Assemble boot code
$(BOOT_OBJ): $(BOOT_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile kernel C (-c = no link)
$(KERNEL_OBJ): $(KERNEL_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile IDT code
$(INTERRUPTS_OBJ): $(INTERRUPTS_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Assemble ISR stub
$(ISR_OBJ): $(ISR_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile exception handlers
$(EXCEPTIONS_OBJ): $(EXCEPTIONS_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Assemble exception stubs
$(EXCEPTIONS_ASM_OBJ): $(EXCEPTIONS_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile PIC code
$(PIC_OBJ): $(PIC_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile IRQ handlers
$(IRQ_OBJ): $(IRQ_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Assemble IRQ stubs
$(IRQ_ASM_OBJ): $(IRQ_ASM) | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile timer driver
$(TIMER_OBJ): $(TIMER_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile keyboard driver
$(KEYBOARD_OBJ): $(KEYBOARD_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile scancode table
$(SCANCODE_OBJ): $(SCANCODE_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile shell
$(SHELL_OBJ): $(SHELL_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile scheduler
$(SCHEDULER_OBJ): $(SCHEDULER_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile kernel tasks
$(KERNEL_TASKS_OBJ): $(KERNEL_TASKS_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile visualizer
$(VISUALIZER_OBJ): $(VISUALIZER_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile physical memory allocator
$(PMEM_OBJ): $(PMEM_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile paging
$(PAGING_OBJ): $(PAGING_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Compile heap allocator
$(HEAP_OBJ): $(HEAP_C) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

# Link final kernel binary
$(KERNEL_BIN): $(BOOT_OBJ) $(KERNEL_OBJ) $(INTERRUPTS_OBJ) $(ISR_OBJ) $(EXCEPTIONS_OBJ) $(EXCEPTIONS_ASM_OBJ) $(PIC_OBJ) $(IRQ_OBJ) $(IRQ_ASM_OBJ) $(TIMER_OBJ) $(KEYBOARD_OBJ) $(SCANCODE_OBJ) $(SHELL_OBJ) $(SCHEDULER_OBJ) $(KERNEL_TASKS_OBJ) $(VISUALIZER_OBJ) $(PMEM_OBJ) $(PAGING_OBJ) $(HEAP_OBJ) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) $(BOOT_OBJ) $(KERNEL_OBJ) $(INTERRUPTS_OBJ) $(ISR_OBJ) $(EXCEPTIONS_OBJ) $(EXCEPTIONS_ASM_OBJ) $(PIC_OBJ) $(IRQ_OBJ) $(IRQ_ASM_OBJ) $(TIMER_OBJ) $(KEYBOARD_OBJ) $(SCANCODE_OBJ) $(SHELL_OBJ) $(SCHEDULER_OBJ) $(KERNEL_TASKS_OBJ) $(VISUALIZER_OBJ) $(PMEM_OBJ) $(PAGING_OBJ) $(HEAP_OBJ) -o $@

# Stage ISO tree exactly like GRUB expects:
# /boot/cheesecake.bin and /boot/grub/grub.cfg
iso: $(KERNEL_BIN)
	mkdir -p $(BUILD_DIR)
	rm -rf $(ISO_ROOT)
	mkdir -p $(ISO_ROOT)/boot/grub
	cp $(KERNEL_BIN) $(ISO_ROOT)/boot/cheesecake.bin
	cp $(GRUB_CFG) $(ISO_ROOT)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $(ISO_IMAGE) -d /usr/lib/grub/i386-pc $(ISO_ROOT)

# Run in QEMU (boot from CD)
run: iso
	$(QEMU) -boot d -cdrom $(ISO_IMAGE)

# Clean generated files
clean:
	rm -rf $(BUILD_DIR)
