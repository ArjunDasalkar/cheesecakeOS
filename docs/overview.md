# Project Overview

## Introduction

CheesecakeOS is a minimal 32-bit x86 operating system kernel written from scratch in C and assembly. Built as both a learning project and a professional portfolio piece, it demonstrates core OS concepts including interrupt handling, memory management, device drivers, and user interaction.

The project focuses on low-level programming and system design, bridging the gap between hardware and software while maintaining readable, well-documented code.

---

## Current Capabilities

### ✅ Hardware & Boot
- **Bootloader:** GRUB Multiboot2, 5-second menu timeout
- **CPU mode:** 32-bit protected mode (x86 i686)
- **Kernel load:** 0x100000 (1MB), identity-mapped via paging
- **Memory:** 16KB stack at kernel base, 256MB total (first 64MB reserved for system)

### ✅ Display & I/O
- **VGA text display:** 80×25 characters, 16-color palette
- **Keyboard:** PS/2 with shift support and key repeat
- **Shift support:** Uppercase letters + symbols (!@#$%^&*)
- **Key repeat:** 500ms initial delay, 50ms repeat interval

### ✅ Interrupt Handling
- **CPU exceptions:** All 32 vectors (0-31) caught and logged
- **PIC (8259):** IRQs remapped to vectors 32-47 (avoids conflicts)
- **Timer (PIT 8254):** 1 kHz, millisecond-accurate tick counter
- **Keyboard (PS/2):** IRQ1, scancode buffering with no data loss

### ✅ Memory Management
- **Physical allocator:** Bitmap-based, 256MB support (4KB pages)
- **Paging:** Page directory/tables, identity-mapping, TLB management
- **Heap allocator:** malloc/free with freelist and automatic page expansion
- **Memory stats:** Available via `memory` shell command

### ✅ Interactive Shell
Five built-in commands:
- `help` — List available commands (with cheesecake puns)
- `time` — Show elapsed time since boot (millisecond precision)
- `memory` — Display memory usage stats
- `clear` — Clear screen and reset cursor
- `reboot` — Halt CPU (graceful shutdown)

---

## Implementation Status

| Layer | Components | Status |
|-------|-----------|--------|
| **Boot** | GRUB, Multiboot2 header, 16KB stack | ✅ Complete |
| **Hardware** | PIT timer, PS/2 keyboard, VGA display | ✅ Complete |
| **Interrupts** | IDT, 32 exceptions, 16 IRQs, PIC remapping | ✅ Complete |
| **Memory** | Physical allocator, paging, heap | ✅ Complete |
| **Drivers** | Timer, keyboard, VGA | ✅ Complete |
| **Shell** | Command parsing, 5 commands | ✅ Complete |
| **Multitasking** | Process scheduling, context switching | ⏳ Planned |
| **Filesystem** | File I/O, disk drivers | ⏳ Planned |
| **Syscalls** | User/kernel boundary, interrupt 0x80 | ⏳ Planned |

---

## Technical Highlights

**Debugging & Testing:**
- Kernel boots in QEMU with `-no-reboot -no-shutdown`
- QEMU interrupt logging (`-d int -D logfile`) for fault diagnosis
- Systematic testing of each component before integration
- Zero crashes or triple-faults in current codebase

**Code Quality:**
- Consistent naming conventions (`ck_` prefix for functions/variables)
- Comprehensive inline comments explaining hardware behavior
- Modular architecture: separate files for each subsystem
- Strict compiler flags: `-Wall -Wextra -Werror` (no warnings)

**Learning Journey:**
- Documented progression from bootloader → interrupts → memory management
- Devlog traces debugging process for interrupt selector mismatch
- Architecture document explains hardware-level decisions
- Each component builds on previous foundations

---

## Build & Run

```bash
# Prerequisites: gcc (32-bit), nasm, grub-pc-bin, xorriso, qemu

# Build
make

# Create bootable ISO
make iso

# Run in QEMU
make run

# Clean
make clean
```

**Build time:** <1 second  
**Kernel size:** ~150KB binary  
**Boot time:** ~2 seconds (QEMU)

---

## Codebase Structure

```
14 sources → 2,500+ lines (code + comments)
- 1 boot assembly (boot.asm)
- 1 kernel entry (ck_kernel.c)
- 1 shell (shell.c)
- 2 interrupt modules (idt.c, pic.c, irq.c + .asm)
- 5 exception/IRQ handlers (.c + .asm)
- 3 driver modules (timer, keyboard, scancode)
- 3 memory modules (pmem, paging, heap)
```

---

## Next Steps (Planned)

1. **Syscall interface** — int 0x80 for user programs
2. **Multitasking** — Process scheduling and context switching
3. **More commands** — echo, cat, ls (filesystem-dependent)
4. **Serial output** — Serial port for debugging
5. **Extended memory** — GRUB multiboot2 mmap parsing
6. **Disk driver** — IDE/ATA for persistent storage
7. **Filesystem** — FAT12 or custom minimal FS

---

## Objectives

- ✅ Understand how an OS interacts with hardware
- ✅ Implement core kernel components (interrupts, memory, drivers)
- ✅ Gain deep experience with systems programming
- ✅ Build a structured, modular, documented codebase
- 🔄 Create a professional portfolio piece

---

## Development Environment

- **Host OS:** Linux / WSL2
- **Compiler:** GCC 13.3+ (with 32-bit multilib)
- **Assembler:** NASM 2.16+
- **Linker:** GNU ld 2.42
- **Bootloader:** GRUB 2.12
- **Emulator:** QEMU 8.2+
- **Build system:** GNU Make 4.3
- **Target:** 32-bit x86 (i686)