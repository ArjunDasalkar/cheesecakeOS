# 🍰 CheesecakeOS

> A minimal 32-bit x86 operating system kernel, built from scratch in C and x86 Assembly.

CheesecakeOS is a hobby kernel project built as a Zoho assignment and a personal deep-dive into systems programming. It implements the core components of a real operating system — memory management, hardware drivers, interrupt handling, and a basic shell — on top of bare x86 hardware, with no underlying OS to fall back on.

This project was built by a competitive programmer learning systems programming for the first time. The code and documentation aim to be honest about that journey — technically precise where it counts, and transparent about what was learned along the way.

---

## Showcase

**Hero demo:** Live multitasking + task monitor + mini game

Try this in QEMU:
```text
help
info
taskmon   (A/D move paddle, Q to exit)
ps
memory
```

What it shows:
- Cooperative multitasking with per-task CPU activity bars
- A mini game running inside the task monitor to make scheduling visible
- Shell commands that expose memory, tasks, and system info

Demo script (2-3 minutes):
```text
Boot QEMU
help
info
taskmon   (A/D move paddle, Q to exit)
ps
memory
```

## Screenshots

Suggested captures (and filenames):
- `01_boot_banner.png` - Boot banner + status line
- `02_help.png` - `help` output with full command list
- `03_info.png` - `info` system overview
- `04_taskmon.png` - Task monitor with mini game + CPU bars
- `05_ps.png` - `ps` task list
- `06_memory.png` - `memory` stats

## Features

- ✅ Bootable kernel via GRUB (Multiboot2)
- ✅ VGA text-mode output (80×25) with color support
- ✅ Interrupt Descriptor Table (IDT) + CPU exception handling (vectors 0-31)
- ✅ PIC remapping and hardware interrupt handling (vectors 32-47)
- ✅ PS/2 Keyboard with shift support and key repeat
- ✅ Programmable Interval Timer (1 kHz tick counter)
- ✅ Physical memory allocator (bitmap-based 4KB pages)
- ✅ x86 paging with identity-mapping
- ✅ Heap allocator (malloc/free with freelist)
- ✅ Cooperative multitasking + scheduler stats
- ✅ Task monitor with CPU bars + mini game demo
- ✅ Interactive command-line shell with 11 commands

---

## Design choices

| Decision | Choice | Reason |
|----------|--------|--------|
| Architecture | x86 32-bit | Simpler than 64-bit, well-documented, great for learning |
| Language | C + x86 Assembly | C for kernel logic, Assembly only where C can't reach |
| Bootloader | GRUB (Multiboot2) | Industry standard, lets us focus on the kernel not the boot process |
| Emulator | QEMU | Fast iteration, no need to flash hardware every test |
| Kernel type | Monolithic | Simpler to implement, all core services run in Ring 0 |

---

## Architecture

```
┌─────────────────────────────────────────┐
│              Kernel Space (Ring 0)      │
│  Shell + Taskmon + Mini Game            │
│  Memory Mgmt │ Interrupts │ Scheduler   │
│  VGA Driver  │  Keyboard  │  Timer      │
├─────────────────────────────────────────┤
│           Hardware (x86, RAM)           │
└─────────────────────────────────────────┘
```

---

## Project structure

```text
cheesecakeOS/
├── boot/
│   ├── boot.asm              # x86 assembly entry point
│   └── grub/
│       └── grub.cfg          # GRUB bootloader config (5s menu timeout)
├── src/
│   ├── kernel/
│   │   ├── ck_kernel.c       # Kernel main (initialization orchestration)
│   │   ├── shell.c           # Interactive shell with 11 commands
│   │   ├── kernel_tasks.c    # Demo tasks + mini game state
│   │   ├── kernel_tasks.h    # Demo task API
│   │   ├── visualizer.c      # Task monitor + mini game display
│   │   └── visualizer.h      # Task monitor API
│   ├── drivers/
│   │   ├── timer.c           # PIT driver (1 kHz timer ticks)
│   │   ├── keyboard.c        # PS/2 keyboard + shift/repeat support
│   │   └── scancode.c        # Scancode-to-ASCII tables (unshifted + shifted)
│   ├── interrupts/
│   │   ├── idt.c             # IDT initialization
│   │   ├── exceptions.c      # CPU exception (0-31) dispatcher
│   │   ├── exceptions.asm    # Exception stub stubs
│   │   ├── pic.c             # PIC remapping (IRQs 32-47)
│   │   ├── irq.c             # IRQ handler dispatcher
│   │   └── irq.asm           # IRQ stubs (16 IRQs)
│   ├── memory/
│   │   ├── pmem.c            # Physical memory allocator (bitmap, 256MB)
│   │   ├── paging.c          # x86 paging (page dir/tables, identity-map)
│   │   └── heap.c            # Heap allocator (malloc/free)
│   └── linker.ld             # Linker script (kernel at 0x100000)
├── docs/
│   ├── overview.md           # Project overview and objectives
│   ├── srs.md                # System requirements and goals
│   ├── architecture.md       # Technical architecture details
│   └── devlog.md             # Development log (debugging, decisions)
├── build/                    # Build artifacts (git-ignored)
├── Makefile                  # Build orchestration
├── README.md                 # This file
├── context.md                # Project context for AI assistants
└── .gitignore
```

---

## Documentation

- Project Overview → [`docs/overview.md`](./docs/overview.md)
- System Requirements → [`docs/srs.md`](./docs/srs.md)
- Architecture → [`docs/architecture.md`](./docs/architecture.md)
- Architecture Diagrams → [`docs/architecture-diagrams.md`](./docs/architecture-diagrams.md)
- Dev Log → [`docs/devlog.md`](./docs/devlog.md)

---

## Getting started

### Prerequisites

Linux or WSL2 (Ubuntu). Install the required tools:

```bash
sudo apt update && sudo apt install \
  build-essential nasm grub-pc-bin grub-common xorriso qemu-system-x86
```

Verify:

```bash
gcc --version && nasm -v && qemu-system-i386 --version
```

### Build & run

```bash
git clone https://github.com/YOUR_USERNAME/cheesecakeOS.git
cd cheesecakeOS
make
make run
```

> Build and run instructions will be finalized as components are implemented.

---

## Implementation roadmap

| Component | Status | Description |
|-----------|--------|-------------|
| Bootloader (GRUB) | ✅ Complete | GRUB config, Multiboot2 header, auto-boot (5s timeout) |
| Kernel entry | ✅ Complete | Boot assembly, `ck_kernel.c`, linker script |
| Build pipeline | ✅ Complete | Makefile: NASM → GCC → LD → ISO (17 sources) |
| VGA text driver | ✅ Complete | Clear screen, write strings, colors (80×25) |
| Kernel boots in QEMU | ✅ Complete | Boots cleanly, no crashes |
| CPU exception handling | ✅ Complete | All 32 exceptions caught and logged |
| PIC remapping & IRQs | ✅ Complete | Master (vec 32-39), Slave (vec 40-47) |
| Timer driver (PIT) | ✅ Complete | 1 kHz tick counter, 1ms granularity |
| Keyboard driver | ✅ Complete | PS/2 scancode buffering, no data loss |
| Scancode-to-ASCII | ✅ Complete | Full US layout, shift support |
| Shift key support | ✅ Complete | Uppercase + symbols (!@#$%^&*) |
| Key repeat | ✅ Complete | 500ms initial delay, 50ms repeat interval |
| Interactive shell | ✅ Complete | 11 commands (help, info, ps, taskmon, etc.) |
| Physical memory allocator | ✅ Complete | Bitmap allocator (256MB, 4KB pages) |
| x86 paging | ✅ Complete | Page directory/tables, identity-mapped kernel |
| Heap allocator | ✅ Complete | malloc/free with freelist + page expansion |
| Multitasking | ✅ Complete | Cooperative scheduling + task stats |
| Task monitor | ✅ Complete | CPU bars + mini game demo |

---

## Learning notes

This project is my first experience with systems programming, coming from a competitive programming background. A few things that were genuinely surprising:

- **There is no safety net.** In normal programming, the OS catches your mistakes. Here, a bad pointer just freezes the machine silently. QEMU makes this survivable — you just restart it.
- **The boot process is wild.** When a PC powers on, the CPU starts in a primitive 16-bit mode from 1978. Getting it into modern 32-bit protected mode requires manual setup before a single line of C can run.
- **C is basically assembly with better syntax** at this level. No standard library, no `printf`, no `malloc` — you implement all of it yourself.

Detailed notes per component live in [`docs/devlog.md`](./docs/devlog.md).

---

## References

- [OSDev Wiki](https://wiki.osdev.org) — the definitive reference for OS development
- [James Molloy's Kernel Tutorial](http://www.jamesmolloy.co.uk/tutorial_html/) — hands-on walkthrough of building a basic kernel
- [The Little OS Book](https://littleosbook.github.io/) — beginner-friendly OS internals
- Intel® 64 and IA-32 Architectures Software Developer's Manual

---

## Status

🔄 In active development

---

## Author

**Arjun Dasalkar**