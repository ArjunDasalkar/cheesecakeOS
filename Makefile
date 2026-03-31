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

# Freestanding 32-bit kernel flags
CFLAGS := -m32 -ffreestanding -Wall -Wextra -Werror -Os -fno-builtin -fno-stack-protector

# NASM output format
ASFLAGS := -f elf32

# Link with custom layout, no stdlib
LDFLAGS := -m elf_i386 -T $(SRC_DIR)/linker.ld -static -nostdlib

# Inputs/outputs
BOOT_ASM := $(BOOT_DIR)/boot.asm
KERNEL_C := $(KERNEL_DIR)/ck_kernel.c
LINKER_SCRIPT := $(SRC_DIR)/linker.ld
GRUB_CFG := $(BOOT_DIR)/grub/grub.cfg

BOOT_OBJ := $(BUILD_DIR)/boot.o
KERNEL_OBJ := $(BUILD_DIR)/ck_kernel.o
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

# Link final kernel binary
$(KERNEL_BIN): $(BOOT_OBJ) $(KERNEL_OBJ) $(LINKER_SCRIPT)
	$(LD) $(LDFLAGS) $(BOOT_OBJ) $(KERNEL_OBJ) -o $@

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
