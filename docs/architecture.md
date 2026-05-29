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

### 1. Boot Assembly (boot/boot.asm) ✅ Complete
- Multiboot2 header (magic 0xE85250D6 + metadata for GRUB)
- Stack setup (16KB BSS section, `ck_stack_top`)
- Entry point (`ck_start`): set ESP, align to 16 bytes, call `ck_main()`
- Halt loop (`cli` + `hlt`) for clean shutdown

### 2. Kernel Main (src/kernel/ck_kernel.c) ✅ Complete
- VGA text driver: clear screen, write strings, color support
- Initialization orchestration: IDT → exceptions → PIC → IRQs → timer → keyboard → paging → heap → shell
- Disables interrupts early, enables after setup complete

### 3. Memory Layer (src/memory/) ✅ Complete
**Physical Allocator (pmem.c)**:
- Bitmap-based allocator (1 bit = 1 page)
- Supports 256MB (65,536 pages × 4KB)
- Marks BIOS (0-384 pages) as reserved
- First-fit allocation and freeing

**Paging (paging.c)**:
- x86 32-bit paging: 10-bit PD index + 10-bit PT index + 12-bit offset
- Page directory (1024 entries) + page tables (up to 256)
- Identity-mapping: virtual addr = physical addr
- Automatic page table allocation, TLB flush via `invlpg`
- CR0/CR3 setup to enable paging

**Heap Allocator (heap.c)**:
- Freelist-based malloc/free
- Per-block header (size + allocated flag)
- Block splitting for efficient allocation
- Automatic page expansion (4KB at a time)
- Fragmentation tracking (total_allocated - total_freed)

### 4. Interrupt Layer (src/interrupts/) ✅ Complete
**IDT Setup (idt.c)**:
- 256 IDT entries, code segment 0x10 (GRUB's kernel CS)
- Trap gates (0x8E) with interrupt disabled flag

**Exception Handlers (exceptions.c + exceptions.asm)**:
- All 32 CPU exceptions (0-31) caught via assembly stubs
- Pushes error code + vector, calls C dispatcher
- Logs exception name and error code, halts

**PIC Remapping (pic.c)**:
- Master (IRQ 0-7) → vectors 32-39
- Slave (IRQ 8-15) → vectors 40-47
- Avoids conflicts with exception vectors (0-31)
- ICW1-ICW4 initialization sequence
- Hard-masking during remap, selective unmask (IRQ0, IRQ1, IRQ2)

**IRQ Handlers (irq.c + irq.asm)**:
- 16 IRQ stubs dispatch to handler table
- Custom callbacks per IRQ (timer on IRQ0, keyboard on IRQ1)
- EOI signaling to master (always) and slave (if IRQ ≥ 8)

### 5. Drivers (src/drivers/) ✅ Complete
**Timer (PIT 8254)**:
- Mode 0x34: counter 0, 16-bit, rate generator
- Reload value 1193 for 1 kHz (1.193182 MHz / 1000)
- IRQ0 handler increments global tick counter
- Millisecond-accurate via `ck_timer_get_ticks()`

**Keyboard (PS/2)**:
- Scancode buffering via circular 256-byte buffer
- IRQ1 handler reads port 0x60
- Shift key detection: scancodes 0x2A (press), 0xAA (release)
- Key release detection: bit 7 set = break code
- Key repeat: 500ms initial delay, 50ms repeat interval
- Scancode-to-ASCII conversion with shift table

**Scancode Tables (scancode.c)**:
- Two tables: unshifted (lowercase) and shifted (UPPERCASE + symbols)
- 256 entries covering US keyboard layout
- Modifier keys return 0 (not printable)

### 6. Shell (src/kernel/shell.c) ✅ Complete
- Command loop with status line (yellow, top row)
- Real-time timer display (updates every ms)
- Five built-in commands:
  - `help`: List commands with cheesecake puns
  - `time`: Show elapsed time (seconds.milliseconds)
  - `memory`: Display RAM + heap stats
  - `clear`: Clear screen, reset cursor
  - `reboot`: Halt CPU gracefully
- Full keyboard support: echo, backspace, shift, key repeat
- VGA scrolling for long output

---

## CPU Privilege Levels

- **Ring 0 (Most Privileged)**: Kernel code — full hardware access
- **Ring 1-2**: Usually unused
- **Ring 3 (Least Privileged)**: User programs — restricted access

Transitions:
- **Kernel → User**: SYSRET/IRET instruction
- **User → Kernel**: INT instruction (syscall) or hardware interrupt

**Current state**: Everything runs in Ring 0 (no ring switching yet).

---

## Paging Details

### x86 32-bit Virtual Address Translation
```
Virtual Address (32-bit):
[31-22: PD Index] [21-12: PT Index] [11-0: Page Offset]
    (10 bits)        (10 bits)         (12 bits)
     
Lookup:
1. Read CR3 (page directory physical address)
2. Index into PD using bits 31-22 → get PT address
3. Index into PT using bits 21-12 → get physical page address
4. Add page offset (bits 11-0) to physical page address
```

### Identity Mapping
- Virtual address 0x00000000 → Physical address 0x00000000
- Virtual address 0x10000000 → Physical address 0x10000000
- Kernel runs transparently (no address translation)
- All memory currently identity-mapped (first 256MB)

### Page Table Entries (PTE) Format
```
Bit  0: Present (1 = mapped, 0 = not present)
Bit  1: Read/Write (1 = writable, 0 = read-only)
Bit  2: User/Supervisor (1 = user-accessible, 0 = kernel-only)
Bits 3-11: Reserved (must be 0)
Bits 12-31: Physical page address (4K-aligned)
```

### TLB Management
- Intel TLB caches virtual → physical mappings
- `invlpg` instruction flushes specific entry
- Called after every page table modification
- INVLPG uses memory operand as TLB flush target (not actually dereferenced)

---

## Interrupt & Exception Flow

### Exception Flow (CPU-generated)
```
CPU detects fault/trap → Hardware pushes state onto stack:
  [ESP+12]: EFLAGS  (flags before exception)
  [ESP+8]:  CS      (code segment)
  [ESP+4]:  EIP     (instruction pointer where fault occurred)
  [ESP]:    Error Code (for some exceptions)

CPU then:
  1. Loads IDT entry based on vector number
  2. Reads segment selector from IDT entry
  3. Loads new instruction pointer from IDT entry
  4. Jumps to handler code
```

### IRQ Flow (Hardware-generated)
```
Hardware device (timer, keyboard) signals interrupt via pin on PIC

PIC converts to vector (32-47) and:
  1. Asserts INT pin on CPU
  2. CPU pushes state (same as exception)
  3. CPU reads vector from PIC
  4. CPU uses vector to index into IDT
  5. Handler code executes
  6. Handler issues EOI to PIC
  7. PIC clears interrupt state
```

### Interrupt Masking
- `cli` (clear interrupt flag) disables hardware IRQs
- `sti` (set interrupt flag) enables hardware IRQs
- Used during kernel initialization to prevent interrupts before IDT ready
- PIC also has mask registers (I/O ports 0x21, 0xA1) for selective masking

---

## Memory Layout (Runtime)

```
Physical Memory (after paging enabled, still identity-mapped)
═══════════════════════════════════════════════════════════════
0x00000000 - 0x000FFFFF   │ Lower 1MB (BIOS, IVT, reserved)
───────────────────────────────────────────────────────────────
0x00400000 - 0x000FFFFF   │ Available (conventional memory, ~6MB)
───────────────────────────────────────────────────────────────
0x000B8000 - 0x000B9FFF   │ VGA Text Buffer (80×25 = 2000 cells, 4KB)
───────────────────────────────────────────────────────────────
0x00100000 (1MB)          ↓ KERNEL LOAD ADDRESS
═══════════════════════════════════════════════════════════════
0x00100000 - 0x00100020   │ .multiboot2 section (32 bytes)
0x00100020 - 0x001XXXXX   │ .text section (code)
0x001XXXXX - 0x001XXXXX   │ .rodata section (strings, constants)
0x001XXXXX - 0x001XXXXX   │ .data section (initialized globals)
0x001XXXXX - 0x00110000   │ .bss section (stack, 16KB)
═══════════════════════════════════════════════════════════════
0x00110000 - 0x10000000   │ Available for future use
═══════════════════════════════════════════════════════════════
(Kernel's heap grows upward from first allocated page)
```

---

## Key Design Decisions

1. **Monolithic Kernel**: All core services in Ring 0
   - Simpler to implement and debug
   - Better performance (no privilege level context switches)
   - Trade-off: one bug can crash entire system

2. **32-bit x86 over 64-bit**:
   - Simpler architecture and instruction set
   - Easier to debug with QEMU
   - Lower memory overhead
   - Still teaches all fundamental OS concepts

3. **GRUB instead of custom bootloader**:
   - Frees us to focus on kernel internals
   - Handles BIOS/UEFI complexity
   - Industry standard (real systems use it)

4. **Identity Mapping (virtual = physical)**:
   - Simplifies initial kernel development
   - Avoids need for address space layout randomization
   - No complex virtual memory setup yet
   - Future: separate kernel/user virtual spaces

5. **Bitmap physical allocator**:
   - Simple and fast (bitwise operations)
   - O(n) worst case (full scan for free page)
   - Sufficient for learning; production would use more sophisticated algorithms

6. **Freelist heap allocator**:
   - First-fit strategy (simple, reasonable performance)
   - Block splitting reduces fragmentation
   - No coalescing (fragmentation possible long-term)
   - Future: improved fragmentation handling

7. **Direct VGA memory access**:
   - 0xB8000 is memory-mapped I/O
   - Each cell = 2 bytes (color | character)
   - Upper byte = color attribute, lower byte = ASCII
   - Allows fast screen updates