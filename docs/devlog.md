# Development Log

## Phase 1: Bootable Kernel (March 31, 2026)

### Overview
Got CheesecakeOS to boot successfully in QEMU and display a message on screen. The entire pipeline works: GRUB loads our Multiboot2 kernel, jumps to assembly entry point, which calls C code that writes directly to VGA text memory.

### Boot Pipeline Architecture

**1. BIOS/QEMU Boot → GRUB**
- BIOS reads El Torito boot record from ISO
- Loads GRUB 2 bootloader (embedded in ISO by `grub-mkrescue`)
- GRUB reads `/boot/grub/grub.cfg` and presents boot menu

**2. GRUB → Kernel Load**
- `grub.cfg` contains menuentry for CheesecakeOS
- GRUB executes: `multiboot2 /boot/cheesecake.bin`
- This tells GRUB to load our kernel at address `0x100000` (1MB)
- GRUB verifies Multiboot2 magic number `0xE85250D6` in kernel binary

**3. Kernel Execution**
- GRUB transfers control to entry point: `ck_start` (in `boot.asm`)
- CPU is in 32-bit protected mode, interrupts disabled
- Stack pointer points to our 16KB stack (`ck_stack_top`)
- Assembly code calls `ck_main()` (kernel C entry point)
- `ck_main()` clears VGA text buffer and writes message

### Key Files
- `boot/boot.asm` — Multiboot2 header + stack setup + entry point
- `src/linker.ld` — Memory layout script (loads kernel at 0x100000, defines sections)
- `src/kernel/ck_kernel.c` — VGA text driver + kernel main
- `boot/grub/grub.cfg` — GRUB boot menu configuration
- `Makefile` — Build orchestration (NASM → GCC → LD → ISO)

### Technical Challenges & Solutions

#### Challenge 1: GRUB Not Auto-Booting
**Problem:** Boot sequence was:
```
BIOS → GRUB prompt (grub>) → MANUAL BOOT REQUIRED
```
GRUB was loading but not executing the menu entry from `grub.cfg`. We had to manually type:
```
multiboot2 /kernel/cheesecake.bin
boot
```

**Root Cause:** The ISO staging layout didn't match GRUB's expected prefix path. When `grub-mkrescue` created the ISO from the `boot/` directory, our `grub.cfg` ended up at a non-standard path. GRUB couldn't find its config because:
1. GRUB's default `prefix=(cd)` (root of ISO)
2. Our config was at `(cd)/grub/grub.cfg` ❌ (non-standard)
3. Kernel reference was at `(cd)/kernel/cheesecake.bin` ❌ (custom path)

GRUB's normal menu loader (`normal.mod`) couldn't auto-execute without proper prefix configuration.

**Solution:** Restructured ISO staging to follow GRUB's standard layout:
```bash
# Before (broken)
boot/
  ├── boot.asm
  ├── cheesecake.bin (copied here)
  └── grub/
      └── grub.cfg

# After (fixed)
build/isodir/
  └── boot/
      ├── cheesecake.bin
      └── grub/
          └── grub.cfg
```

Updated Makefile to:
1. Create proper staging tree under `build/isodir/`
2. Copy kernel to `build/isodir/boot/cheesecake.bin`
3. Copy config to `build/isodir/boot/grub/grub.cfg`
4. Pass this to `grub-mkrescue`: `grub-mkrescue -o cheesecake.iso build/isodir`

Result: GRUB now finds its config at standard `/boot/grub/grub.cfg` and auto-executes the menuentry ✅

#### Challenge 2: C Code Calling Conventions
**Problem:** Kernel hung silently after GRUB loaded it. Assembly was correct (proven by inline VGA test), but calling `ck_main()` from assembly was breaking.

**Root Cause:** x86 32-bit System V calling convention requires:
- Stack pointer aligned to 16-byte boundary at call instruction
- We aligned at `ck_start` with `and esp, 0xFFFFFFF0`, but this left stack 4 bytes off when `call ck_main` executed
- x86-64 and modern GCC expect `(esp % 16) == 0` **before** the call, meaning `(esp % 16) == 12` **inside** the function
- Violated this and GCC's stack-smashing code in `ck_kernel.o` crashed

**Solution:** Used freestanding compiler flags to disable stack canaries:
```makefile
CFLAGS := -m32 -ffreestanding -fno-stack-protector ...
```
Also ensured linker script placed `.multiboot2` section first (within 32KB of binary start) so GRUB's Multiboot2 parser could find the magic number.

#### Challenge 3: Compiler Strictness on Bare Metal
**Problem:** Inline assembly for serial I/O was triggering:
```
error: array subscript 0 is outside array bounds of 'volatile unsigned char[0]'
```
Compiler was treating memory-mapped I/O ports as array indexing and rejecting it.

**Solution:** Use explicit pointer variables instead of inline casts:
```c
volatile unsigned char *port = (volatile unsigned char *)0x3F8;
*port = data;  // Works, compiler understands
```
Eventually simplified to just VGA text mode output (doesn't need serial for now).

### Build Pipeline Details

**Step 1: Compile Assembly**
```bash
nasm -f elf32 boot/boot.asm -o build/boot.o
```
Outputs ELF 32-bit object with sections: `.multiboot2`, `.bss` (stack), `.text` (code).

**Step 2: Compile C Kernel**
```bash
gcc -m32 -ffreestanding ... src/kernel/ck_kernel.c -c -o build/ck_kernel.o
```
Nolibc compilation. Produces ELF object with `.text`, `.rodata`, `.data`, `.bss`.

**Step 3: Link Objects**
```bash
ld -m elf_i386 -T src/linker.ld -static -nostdlib build/boot.o build/ck_kernel.o -o build/cheesecake.bin
```
Linker script controls layout:
- Entry point: `ck_start` from `boot.o`
- Load address: `0x100000` (1MB)
- Section order: `.multiboot2` first (GRUB parser requirement), then `.text`, `.rodata`, `.data`, `.bss`

**Step 4: Create ISO**
```bash
grub-mkrescue -o build/cheesecake.iso -d /usr/lib/grub/i386-pc build/isodir
```
Embeds GRUB bootloader + our kernel into an El Torito bootable ISO.

**Step 5: Boot in QEMU**
```bash
qemu-system-i386 -boot d -cdrom build/cheesecake.iso
```
QEMU emulates BIOS, which boots from CD (GRUB), which loads kernel.

### VGA Text Mode Output

Kernel writes directly to VGA text memory at physical address `0xB8000`:
```c
#define VGA_BUFFER ((unsigned short *)0xB8000)
VGA_BUFFER[row * 80 + col] = (color << 8) | character;
```

Each cell is 16 bits:
- **Lower byte:** ASCII character
- **Upper byte:** color attribute (foreground in lower nibble, background in upper nibble)
  - `0x0F` = white on black (default)

Current kernel just clears screen + prints "CheesecakeOS served fresh..." then halts forever.

### Lessons Learned

1. **Bootloaders are finicky.** GRUB works, but the ISO layout and prefix paths matter enormously. Off-by-one mistakes result in grub> prompt instead of boot.
2. **Testing early & often with simple cases saves hours.** I tested pure assembly VGA output first (verified boot pipeline works), then gradually added C.
3. **System V ABI is strict.** Stack alignment, register preservation, calling conventions — all must be exact. Even a 4-byte misalignment breaks GCC's generated code silently.
4. **Compiler flags for bare metal are non-negotiable.** `-ffreestanding` and `-fno-stack-protector` are essential to prevent libc dependencies and UB assumptions.
5. **Linker scripts control everything.** The binary layout, section placement, and entry point are all determined here, not by the compiler.

### Current Status
✅ **Phase 1 Complete:** Kernel boots, runs C code, writes to VGA, halts cleanly in QEMU.

Next: Interrupt handling (IDT, ISR, PIC remapping) → PS/2 keyboard driver.