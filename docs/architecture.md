# Technical Architecture

## Boot Sequence

```
1. Power On
   ↓
2. BIOS/UEFI → El Torito Boot Record (ISO)
   ↓
3. GRUB Bootloader Loads
   - Switches from 16-bit real mode → 32-bit protected mode
   - Parses ISO 9660 filesystem
   - Reads /boot/grub/grub.cfg
   ↓
4. GRUB Menu (timeout=0, so skip display)
   - Executes: multiboot2 /boot/cheesecake.bin
   - Verifies Multiboot2 magic: 0xE85250D6
   ↓
5. GRUB Loads Kernel
   - Places kernel at physical address 0x100000 (1MB)
   - Sets CPU to 32-bit protected mode
   - Disables interrupts
   - Sets up basic GDT (if needed)
   ↓
6. GRUB → Kernel Transfer
   - Jumps to entry point: ck_start (from boot.asm)
   - ESP points to kernel's stack (top of 16KB stack buffer)
   ↓
7. Kernel Initialization (boot.asm: ck_start)
   - Align stack to 16 bytes (x86 SysV requirement)
   - Call ck_main() (kernel C code)
   ↓
8. Kernel Main (ck_kernel.c: ck_main)
   - Initialize VGA text driver
   - Print boot message
   - Loop forever (CLI + HLT)
```

---

## Memory Layout

```
  Physical Memory
  ═══════════════════════════════════════════════════════════════
  0x00000000 - 0x000FFFFF     │ Lower 1MB (BIOS, interrupts, etc)
  ───────────────────────────────────────────────────────────────
  0x00000000 - 0x000003FF     │ IVT (Interrupt Vector Table, real mode)
  0x00000400 - 0x000004FF     │ BIOS Data Area
  0x00000500 - 0x0009FBFF     │ Available (conventional memory)
  0x0009FC00 - 0x0009FFFF     │ BIOS Extended RAM area
  0x000F0000 - 0x000FFFFF     │ BIOS ROM
  ───────────────────────────────────────────────────────────────
  0xB8000 - 0xB9FFF           │ VGA Text Buffer (80×25, 4KB)
                              │ (I/O mapped, not real RAM)
  ───────────────────────────────────────────────────────────────
  0x100000 (1MB)              ↓ KERNEL LOAD ADDRESS
  ═══════════════════════════════════════════════════════════════
  0x100000 - 0x10????         │ .multiboot2 section (32 bytes)
  0x10???? - 0x10????         │ .text section (code)
  0x10???? - 0x10????         │ .rodata section (read-only data)
  0x10???? - 0x10????         │ .data section (initialized data)
  0x10???? - 0x100000 + 4KB   │ .bss section (kernel stack 16KB)
  ═══════════════════════════════════════════════════════════════
  0x00000000 (after paging enabled, maps to physical above)
```

**Why 0x100000?**
- Multiboot spec standard for 32-bit kernels
- Avoids BIOS data, IVT, and conventional memory complications
- Allows large kernels without hitting lower 1MB constraints
- Clean separation between bootloader and kernel space

---

## Linker Script (src/linker.ld)

```ld
/* Entry point for kernel */
ENTRY(ck_start)

/* Output format */
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")

SECTIONS {
    . = 0x100000;  /* Load kernel at 1MB */

    .multiboot2 : {  /* MUST be first (within 32KB for GRUB search) */
        KEEP(*(.multiboot2))
    }

    .text : {        /* Executable code */
        KEEP(*(.text))
        *(.text*)
    }

    .rodata : {      /* Read-only data (strings, constants) */
        *(.rodata)
        *(.rodata*)
    }

    .data : {        /* Initialized global/static data */
        *(.data)
        *(.data*)
    }

    .bss : {         /* Uninitialized data (stack, uninitialized globals) */
        KEEP(*(.bss))
        *(.bss*)
        *(COMMON)
    }
}
```

**Key points:**
- `.multiboot2` first so GRUB can find magic number within first 32KB
- `ck_start` as ENTRY so linker knows where execution begins
- Everything loaded at 0x100000 (physical address, not virtual yet)

---

## System Architecture Layers

```
┌──────────────────────────────────────────┐
│       User Mode (Ring 3) — Future        │
│   (Shell, user programs running here)    │
├──────────────────────────────────────────┤
│        Syscall Interface (int 0x80)      │
├──────────────────────────────────────────┤
│         Kernel Mode (Ring 0)             │
├──────────────────────────────────────────┤
│  Core Services:                          │
│  • Memory Management (paging, heap)      │
│  • Interrupt/Exception Handling (IDT)    │
│  • Process/Task Scheduling               │
│  • File System (future)                  │
├──────────────────────────────────────────┤
│  Drivers:                                │
│  • VGA Text Driver (current)             │
│  • Keyboard Driver (PS/2, future)        │
│  • Timer Driver (PIT/APIC, future)       │
│  • Disk Driver (ATA/IDE, future)         │
├──────────────────────────────────────────┤
│         Hardware (x86 CPU, RAM)          │
└──────────────────────────────────────────┘
```

---

## Component Overview

### 1. Boot Assembly (boot/boot.asm)
- Multiboot2 header (magic + metadata for GRUB)
- Stack setup (16KB BSS section)
- Entry point (`ck_start`): minimal setup, call `ck_main()`
- Halt loop for when kernel returns

### 2. Kernel Main (src/kernel/ck_kernel.c)
- VGA text driver: clear screen, write strings
- Currently: just prints message and halts
- Future: call subsystem inits (memory, interrupts, drivers)

### 3. Memory Layer (src/memory/) — Planned
- Physical memory allocator (bitmap-based frame allocator)
- Paging setup (page tables, virtual address translation)
- Heap allocator (`ck_malloc`, `ck_free`)

### 4. Interrupt Layer (src/interrupts/) — Planned
- Interrupt Descriptor Table (IDT) setup
- Exception handlers (0x00-0x1F)
- Hardware interrupt handlers (0x20+)
- PIC (Programmable Interrupt Controller) remapping

### 5. Drivers (src/drivers/) — Planned
- **VGA**: text output (current: inline in kernel)
- **Keyboard**: PS/2 keyboard via IRQ1
- **Serial**: COM1 for debug logging
- **Timer**: PIT for scheduling and delays

### 6. Shell — Planned
- Command parser
- Built-in commands (help, reboot, etc)
- Future: fork/exec for user programs

---

## CPU Privilege Levels

- **Ring 0 (Most Privileged)**: Kernel code — full hardware access
- **Ring 1-2**: Usually unused
- **Ring 3 (Least Privileged)**: User programs — restricted access

Transitions:
- **Kernel → User**: SYSRET/IRET instruction
- **User → Kernel**: INT instruction (syscall) or hardware interrupt

Currently, everything runs in Ring 0 (no ring switching yet).

---

## Key Design Decisions

1. **Monolithic Kernel**: All core services in Ring 0
   - Simpler to implement
   - Better performance (no privilege level context switches)
   - Less isolation (one bug crashes everything)

2. **32-bit x86 over 64-bit**:
   - Simpler architecture
   - Easier to debug
   - Lower memory overhead
   - Still teaches all fundamentals

3. **GRUB instead of custom bootloader**:
   - Frees us to focus on kernel internals
   - Handles BIOS/UEFI complexity
   - Industry standard (real systems use it)

4. **Direct VGA memory access**:
   - No driver abstraction layer yet
   - Fast iteration during development
   - Will abstract into driver layer later

5. **No paging enabled yet**:
   - Running in physical address space
   - Simpler initial kernel
   - Paging layer will be non-trivial (page fault handlers, TLB management)