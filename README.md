# 🍰 CheesecakeOS

> A minimal 32-bit x86 operating system kernel, built from scratch in C and x86 Assembly.

CheesecakeOS is a hobby kernel project built as a Zoho assignment and a personal deep-dive into systems programming. It implements the core components of a real operating system — memory management, hardware drivers, interrupt handling, and a basic shell — on top of bare x86 hardware, with no underlying OS to fall back on.

This project was built by a competitive programmer learning systems programming for the first time. The code and documentation aim to be honest about that journey — technically precise where it counts, and transparent about what was learned along the way.

---

## Features (Planned)

- Bootable kernel via GRUB (Multiboot2)
- VGA text-mode output (80×25)
- Interrupt Descriptor Table (IDT) + hardware interrupt handling
- PS/2 Keyboard and timer drivers
- Memory management (physical allocator, paging, heap)
- Basic interactive command-line shell
- (Optional) Basic multitasking support

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
│              User Space (Ring 3)        │
│         Shell  │  Standard Library      │
├─────────────────────────────────────────┤
│           Syscall Interface             │  ← int 0x80
├─────────────────────────────────────────┤
│              Kernel Space (Ring 0)      │
│  Memory Mgmt │ Interrupts │ Scheduler   │
│  VGA Driver  │  Keyboard  │  Serial     │
├─────────────────────────────────────────┤
│           Hardware (x86, RAM)           │
└─────────────────────────────────────────┘
```

---

## Project structure

```text
cheesecakeOS/
├── boot/
│   └── grub/
│       └── grub.cfg          # GRUB bootloader config
├── src/
│   ├── kernel/
│   │   └── ck_kernel.c       # Kernel entry point
│   ├── drivers/              # VGA, keyboard, serial drivers
│   ├── memory/               # Physical allocator, paging, heap
│   ├── interrupts/           # IDT, ISR, IRQ handlers
│   └── linker.ld             # Linker script — memory layout
├── docs/
│   ├── overview.md           # Project overview
│   ├── srs.md                # System requirements
│   ├── architecture.md       # Detailed architecture notes
│   └── devlog.md             # Development log
├── build/                    # Compiled output (git-ignored)
├── Makefile
├── README.md
└── .gitignore
```

---

## Documentation

- Project Overview → [`docs/overview.md`](./docs/overview.md)
- System Requirements → [`docs/srs.md`](./docs/srs.md)
- Architecture → [`docs/architecture.md`](./docs/architecture.md)
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
| Bootloader (GRUB) | ✅ Setup | GRUB config, Multiboot2 header |
| Kernel entry | 🔄 In progress | Boot assembly, `ck_kernel.c` |
| VGA driver | ⏳ Planned | Text-mode display (80×25) |
| Interrupt handling | ⏳ Planned | IDT, ISR, PIC remapping |
| Keyboard driver | ⏳ Planned | PS/2 keyboard via IRQ1 |
| Physical memory manager | ⏳ Planned | Bitmap frame allocator |
| Heap allocator | ⏳ Planned | `ck_malloc` / `ck_free` |
| Paging | ⏳ Planned | Page directory/tables, virtual memory |
| Shell | ⏳ Planned | Basic CLI with built-in commands |

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