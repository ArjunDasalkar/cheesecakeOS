# Software Requirements Specification (SRS)

## Executive Summary

CheesecakeOS is a minimal 32-bit x86 operating system kernel designed to demonstrate core OS concepts through direct hardware interaction. The system prioritizes educational clarity and clean architecture over feature completeness.

---

## 1. Functional Requirements

### 1.1 Boot & Initialization
- **FR1.1**: System shall boot via GRUB Multiboot2 bootloader at 0x100000 (1MB)
- **FR1.2**: Kernel shall set up 16KB stack before any C code execution
- **FR1.3**: Kernel shall disable interrupts during initialization
- **FR1.4**: Kernel shall initialize all subsystems (IDT, PIC, timer, keyboard, memory) before enabling interrupts
- **Status**: ✅ Complete

### 1.2 CPU Exception Handling
- **FR2.1**: Kernel shall catch all 32 CPU exceptions (vectors 0-31)
- **FR2.2**: Kernel shall log exception details (vector, error code, instruction pointer)
- **FR2.3**: Kernel shall halt gracefully on unrecoverable exceptions
- **Status**: ✅ Complete

### 1.3 Hardware Interrupt Handling
- **FR3.1**: Kernel shall remap PIC to avoid conflict with exceptions
  - Master IRQs → vectors 32-39
  - Slave IRQs → vectors 40-47
- **FR3.2**: Kernel shall provide configurable IRQ handlers
- **FR3.3**: Kernel shall send EOI (End-Of-Interrupt) after handling each IRQ
- **Status**: ✅ Complete

### 1.4 Display & Output
- **FR4.1**: VGA text driver shall display 80×25 character grid
- **FR4.2**: VGA driver shall support 16 colors (0x0-0xF, low 4 bits = foreground)
- **FR4.3**: VGA driver shall support string output with newlines and scrolling
- **FR4.4**: VGA driver shall display real-time status line (yellow, row 0)
- **Status**: ✅ Complete

### 1.5 Timer Driver
- **FR5.1**: System shall provide millisecond-accurate tick counter
- **FR5.2**: PIT 8254 shall generate 1 kHz timer interrupt (IRQ0)
- **FR5.3**: Global counter shall increment every millisecond
- **FR5.4**: Shell shall display elapsed time (seconds.milliseconds)
- **Status**: ✅ Complete

### 1.6 Keyboard Input
- **FR6.1**: Keyboard driver shall accept PS/2 keyboard via IRQ1
- **FR6.2**: Driver shall buffer scancodes in circular 256-byte buffer
- **FR6.3**: Driver shall provide scancode-to-ASCII conversion (US layout)
- **FR6.4**: Driver shall support shift key for uppercase and symbols
- **FR6.5**: Driver shall implement key repeat (500ms initial, 50ms interval)
- **FR6.6**: Shell shall echo typed characters to screen
- **FR6.7**: Shell shall support backspace for editing
- **Status**: ✅ Complete

### 1.7 Interactive Shell
- **FR7.1**: System shall provide command-line interface with prompt ("> ")
- **FR7.2**: Shell shall accept and parse user commands
- **FR7.3**: Shell shall implement at least 5 built-in commands:
  - `help`: List available commands
  - `time`: Display elapsed time
  - `memory`: Display memory statistics
  - `clear`: Clear screen
  - `reboot`: Gracefully halt CPU
- **FR7.4**: Shell shall echo user input in real-time
- **FR7.5**: Shell shall display cheesecake-themed puns
- **Status**: ✅ Complete

### 1.8 Memory Management
- **FR8.1**: Physical memory allocator shall manage 4KB pages (up to 256MB)
- **FR8.2**: Allocator shall use bitmap for tracking page allocation
- **FR8.3**: System shall implement x86 paging with page directory/tables
- **FR8.4**: Paging shall initially identity-map kernel (virtual = physical)
- **FR8.5**: Heap allocator shall provide malloc/free functionality
- **FR8.6**: Heap shall automatically expand by allocating physical pages
- **FR8.7**: Shell `memory` command shall display:
  - Total available RAM (bytes)
  - Used and free pages
  - Heap allocation statistics
- **Status**: ✅ Complete

### 1.9 Debugging & Testing
- **FR9.1**: System shall boot cleanly without crashes in QEMU
- **FR9.2**: All components shall compile with strict flags (-Wall -Wextra -Werror)
- **FR9.3**: Exception handlers shall print diagnostic information
- **Status**: ✅ Complete

---

## 2. Non-Functional Requirements

### 2.1 Performance
- **NFR1.1**: Kernel boot time shall be <2 seconds in QEMU
- **NFR1.2**: Shell responsiveness shall be <50ms (keyboard input to screen echo)
- **NFR1.3**: Timer tick shall have <1ms granularity error
- **NFR1.4**: Page allocation shall be O(n) worst case (acceptable for 256MB)
- **Status**: ✅ Meets or exceeds targets

### 2.2 Reliability
- **NFR2.1**: No triple-faults or kernel crashes in normal operation
- **NFR2.2**: Interrupts shall not be lost due to masking/race conditions
- **NFR2.3**: Keyboard buffer shall not drop input during normal operation
- **NFR2.4**: Memory allocator shall not corrupt heap metadata
- **Status**: ✅ No observed failures

### 2.3 Code Quality
- **NFR3.1**: All code shall compile with `-Wall -Wextra -Werror` (zero warnings)
- **NFR3.2**: Functions shall have clear inline comments explaining behavior
- **NFR3.3**: Naming conventions shall be consistent (`ck_` prefix)
- **NFR3.4**: Code shall follow modular architecture (separate files per subsystem)
- **Status**: ✅ Complete

### 2.4 Maintainability
- **NFR4.1**: Each component shall have clear API (header files)
- **NFR4.2**: Kernel initialization sequence shall be documented
- **NFR4.3**: Hardware-specific details shall be explained in comments
- **Status**: ✅ Well-documented

### 2.5 Portability
- **NFR5.1**: System shall target 32-bit x86 architecture (i686)
- **NFR5.2**: Code shall compile with GCC 12+ with 32-bit multilib
- **NFR5.3**: System shall run on QEMU i386 emulator
- **NFR5.4**: System shall use only standard build tools (gcc, nasm, ld, grub-mkrescue)
- **Status**: ✅ Verified on Linux/WSL2

---

## 3. Technical Constraints

### 3.1 Language & Environment
- Written entirely in C (kernel) and x86 Assembly (boot, stubs)
- No C standard library (libc) — only `-ffreestanding`
- No external dependencies beyond GNU toolchain + GRUB
- Target: 32-bit protected mode x86

### 3.2 Hardware Constraints
- Minimum 4MB RAM (typical QEMU default is 128MB)
- VGA text mode (0xB8000, 80×25)
- PS/2 keyboard support
- PIT 8254 timer at 1.193182 MHz base frequency

### 3.3 Architectural Constraints
- Monolithic kernel (all code runs in Ring 0)
- Identity-mapped virtual memory (no ASLR or address translation)
- No user-space separation (future feature)
- No process scheduling yet (single execution context)

### 3.4 Build Constraints
- Build time <1 second (excluding GRUB ISO creation)
- Final kernel binary <1MB (typical: ~150KB)
- ISO image <10MB (with GRUB + kernel)

---

## 4. Known Limitations & Future Work

### 4.1 Current Limitations
- No syscall interface (user/kernel boundary)
- No multitasking or process scheduling
- No filesystem or persistent storage
- No demand paging (entire kernel identity-mapped)
- No user-space programs
- Limited memory introspection (basic stats only)
- No support for CPU features (FPU, SSE, etc.)

### 4.2 Planned Features
- Syscall interface (int 0x80)
- Process scheduling and context switching
- Filesystem (FAT12 or custom minimal FS)
- Serial output for debugging
- Extended GRUB Multiboot2 memory map parsing
- Disk driver (IDE/ATA)
- User-space program loader

---

## 5. Acceptance Criteria

- [x] Kernel boots cleanly in QEMU without crashes
- [x] All 32 CPU exceptions caught and logged
- [x] All 16 hardware IRQs handled correctly
- [x] Timer tick accurate to 1ms
- [x] Keyboard input reliable (no dropped characters)
- [x] Shell responsive to user commands
- [x] Memory allocator functional (malloc/free works)
- [x] Paging enabled without issues
- [x] All components compile with strict compiler flags
- [x] Comprehensive documentation provided

---

## 6. Success Metrics

| Metric | Target | Current | Status |
|--------|--------|---------|--------|
| Boot time | <2s | ~1.5s | ✅ |
| Shell responsiveness | <50ms | ~10ms | ✅ |
| Code warnings | 0 | 0 | ✅ |
| Exception coverage | 100% | 32/32 | ✅ |
| IRQ coverage | 100% | 16/16 | ✅ |
| Memory allocations | Reliable | Yes | ✅ |
| Keyboard input | No drops | Verified | ✅ |
| Documentation | Complete | Yes | ✅ |