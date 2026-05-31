# ZOHO SETU PROJECT REPORT

## 10. Baseline Kernel OS with Drivers, Memory Management and Basic Shell (CheesecakeOS)


### Submitted By:
**Name:** Arjun Dasalkar  
**College:** Government College of Engineering, Aurangabad  
**Class:** Third Year  
**Department:** Computer Science and Engineering  
**Roll No:** BT23F05F014  
**GitHub Repository:** https://github.com/ArjunDasalkar/cheesecakeOS

---

# Problem Statement
Create a minimal Linux kernel with core components like drivers, memory 
management, and basic shell. 

---

# Technologies Used
ComponentTechnologyProgramming LanguageCLow-Level Programmingx86 Assembly (NASM)Target Architecture32-bit x86 (i686)BootloaderGRUB (Multiboot2)EmulatorQEMUBuild SystemGNU MakeLinkerGNU ldCompilerGCCDevelopment EnvironmentLinux / WSL2
---

# Abstract
CheesecakeOS is a minimal educational operating system kernel developed from scratch for the 32-bit x86 architecture. The project demonstrates core operating system concepts including hardware interrupt handling, memory management, device driver development, task scheduling, and shell-based user interaction.

The kernel boots through GRUB using the Multiboot2 specification and initializes fundamental subsystems such as the Interrupt Descriptor Table (IDT), Programmable Interrupt Controller (PIC), PIT timer, keyboard input handling, physical memory management, paging, and heap allocation.

While the original problem statement required a kernel with drivers, memory management, and a basic shell, CheesecakeOS extends beyond those requirements by implementing a cooperative multitasking scheduler, a live task monitor, and a real-time demo game that visualizes scheduling behavior.

---

# Objectives
This project was designed with the following objectives:

### Objective 1: Develop a Bootable x86 Kernel
Implement a kernel capable of booting successfully using GRUB.

### Objective 2: Implement Hardware Driver Support
Develop drivers for keyboard input and timer interrupts.

### Objective 3: Implement Memory Management
Provide physical memory allocation, virtual memory paging, and dynamic heap allocation.

### Objective 4: Build a Command-Line Shell
Allow users to interact with the operating system through shell commands.

### Objective 5: Implement Interrupt Handling
Handle CPU exceptions and hardware interrupts through the IDT and PIC.

### Objective 6: Demonstrate Operating System Concepts
Provide practical implementations of scheduling, memory management, and hardware communication.

### Objective 7: Extend Beyond Requirements
Implement multitasking and live monitoring utilities to showcase advanced OS functionality.

---

# Design Choice: Why 32-bit Instead of 64-bit?
CheesecakeOS targets 32-bit x86 (i686) intentionally for these reasons:

1. **Lower complexity**: 32-bit protected mode avoids the additional boot steps and paging structures required for long mode.  
2. **Faster iteration**: Smaller code paths make debugging and reasoning significantly easier.  
3. **Better learning alignment**: The goal is deep systems understanding, not breadth. 32-bit is well-documented and easier to bring up correctly.  
4. **Clear architecture focus**: Using 32-bit allows emphasis on interrupts, paging, memory allocators, and scheduling without long-mode overhead.  

This decision enabled a faster path to a stable, demonstrable kernel and a stronger final system.

---

# System Architecture Diagram
**[INSERT SYSTEM ARCHITECTURE DIAGRAM HERE]**

Suggested architecture:

User Commands -> Shell -> Kernel Services -> Drivers / Memory Manager / Scheduler -> Hardware

---

# System Boot Sequence
**[INSERT SCREENSHOT HERE]**

*Figure 1: CheesecakeOS boot process showing initialization of core kernel subsystems including interrupts, timer, keyboard driver, memory manager, and scheduler.*

---

# Shell Command Interface
**[INSERT SCREENSHOT HERE]**

*Figure 2: Help command displaying all available shell commands provided by CheesecakeOS.*

Available commands include:

- help
- uptime
- time
- echo
- info
- ps
- memory
- tasks
- taskmon
- history
- clear
- reboot

---

# System Information
**[INSERT SCREENSHOT HERE]**

*Figure 3: System information command displaying architecture, memory usage, interrupt configuration, timer configuration, and scheduler statistics.*

---

# Memory Management and Process Status
**[INSERT SCREENSHOT HERE]**

*Figure 4: Memory and process inspection commands showing physical memory statistics, heap allocation status, and active task information.*

---

# Scheduler Monitor (State 1)
**[INSERT SCREENSHOT HERE]**

*Figure 5: Real-time task scheduler monitor showing CPU distribution among active tasks and live scheduling statistics.*

---

# Scheduler Monitor (State 2)
**[INSERT SCREENSHOT HERE]**

*Figure 6: Interactive scheduler demonstration showing task execution state changes during runtime.*

---

# How CheesecakeOS Satisfies the Problem Statement

## 1. Kernel Implementation
CheesecakeOS implements a standalone kernel that boots via GRUB and initializes all required operating system subsystems.

---

## 2. Device Drivers

### Keyboard Driver
- Reads PS/2 scancodes  
- Converts to ASCII  
- Supports Shift and key repeat  

### Timer Driver
- PIT at 1 kHz  
- Maintains uptime ticks  
- Supports scheduling timing  

### VGA Text Interface
- 80x25 VGA output  
- Colored shell output  
- Status line + interactive prompt  

---

## 3. Memory Management

### Physical Memory Manager
- Bitmap allocator  
- 256 MB / 65,536 pages  
- 4 KB page size  

### Virtual Memory Paging
- Identity-mapped paging  
- Page directory + page tables  
- Foundation for isolation  

### Heap Allocator
- Freelist allocator  
- Dynamic allocation & free  
- Kernel heap expansion  

---

## 4. Basic Shell
The shell provides:
- System inspection  
- Memory stats  
- Task monitoring  
- Administrative commands  
- Reboot support  

---

## 5. Interrupt Handling

### CPU Exceptions
All 32 exceptions handled with logging and safe halt.

### Hardware Interrupts
PIC remapped and IRQs configured:
- Timer IRQ  
- Keyboard IRQ  

---

# Hero Feature: Taskmon + Interactive Scheduler Game

## Overview
The most unique feature in CheesecakeOS is `taskmon`, a live task monitor that visually demonstrates multitasking. It is paired with a small interactive game that runs inside the scheduler, proving real concurrent task execution.

This makes scheduling behavior visible to anyone watching the demo, which is rare in student kernels.

---

## What Taskmon Shows
- Live CPU activity bars for each task (per-interval usage)  
- Task counters updated in real time  
- Total context switches  
- Active task count  

---

## The Scheduler Game
Inside `taskmon`, a small "Kitchen Bounce" mini game runs:

- **Task 0 (Physics):** updates ball position + collisions  
- **Task 1 (Input):** moves paddle using A/D keys  
- **Task 2 (Render Load):** adds variable workload  

Each task produces different workload patterns, so CPU bars visibly shift and react.  
This turns scheduling into a visible, interactive demonstration.

---

## Why This Matters
Most kernels only show task counters or logs. CheesecakeOS makes scheduling observable, which:

- Proves multitasking at runtime  
- Demonstrates inter-task cooperation  
- Makes the demo more impressive for reviewers  

---

# Features Beyond Problem Statement

## 1. Cooperative Multitasking Scheduler
- Task creation  
- Round-robin scheduling  
- Scheduler statistics  

## 2. Task Monitoring Utilities
- Live CPU bars  
- Task counters  
- Context switch tracking  

## 3. Interactive Scheduler Demonstration
- Game shows real concurrency  
- Input + physics + workload in parallel  

## 4. Advanced Shell Utilities
- `info`, `ps`, `tasks`, `history`, `taskmon`  
- Debug-friendly commands  

---

# Challenges Faced
- Interrupt configuration + IDT correctness  
- Memory paging correctness  
- Scheduler integration without breaking shell responsiveness  
- Making visualization stable and readable  

---

# Debugging: The Biggest Challenge in Kernel Work
Debugging in bare-metal kernel projects is the hardest part of the work. There is no standard library, no OS safety net, and a single wrong pointer or descriptor can silently crash or reboot the system. Most failures present as black screens, freezes, or unexpected resets. That makes diagnosis slow, methodical, and highly dependent on small experiments.

CheesecakeOS went through multiple debugging cycles documented in the dev log. The most instructive ones are summarized below.

## 1. Day-One GRUB Bootloading Failures
The first major blocker was the kernel not auto-booting. QEMU dropped straight into the GRUB prompt instead of running the menu entry. The root cause was ISO staging layout: `grub.cfg` and the kernel binary were not in GRUB’s expected `/boot/grub/grub.cfg` and `/boot/cheesecake.bin` locations. The solution was to restructure the build output into the standard GRUB layout and rebuild the ISO using `grub-mkrescue`. Once the staging tree matched GRUB’s expectations, the boot sequence became reliable.

This early problem was a reminder that boot tooling is just as fragile as kernel code, and even a perfect kernel binary will fail if the bootloader cannot find its configuration.

## 2. Kernel Entry + Calling Convention Issues
After GRUB loading worked, the kernel would hang immediately after entry. The assembly entry was correct, but the C call into `ck_main` failed due to stack alignment expectations. The fix required freestanding compiler flags (including `-fno-stack-protector`) and careful alignment of the stack before calling into C. This reinforced a key lesson: in a freestanding environment, calling conventions and compiler assumptions matter just as much as the code itself.

## 3. Interrupt System Failures (The "Silent Reset" Class)
The hardest class of bugs involved interrupts. Several issues compounded:
- **IDT gate selector mismatch:** The IDT used the wrong code segment selector, causing every interrupt to trigger a General Protection Fault.
- **Early IRQ during boot:** Timer IRQs fired before the IDT and PIC were fully initialized, leading to faults before the kernel was stable.
- **PIC masking mistakes:** IRQs remained masked, so timer and keyboard interrupts never fired.

The fixes involved:
- Forcing interrupts off (`cli`) during early boot
- Correcting the CS selector used in IDT gate entries
- Temporarily masking the PIC during remap and only unmasking IRQ0/IRQ1/IRQ2 afterward

These issues were diagnosed using QEMU’s interrupt logging (`-d int`) and by running with `-no-reboot -no-shutdown` to prevent automatic resets. The devlog captures the exact vector numbers and error codes used to find the faulty IDT gate configuration.

## 4. Timer + Shell Loop Timing Bugs
Once interrupts worked, the status line timer occasionally froze. The root cause was ordering in the shell loop: task execution and status redraw were competing, and a long-running task could prevent the timer display from updating. The fix was to make demo tasks return after a single step and to reorder the loop so scheduler passes happen before status draws. The timer then updated consistently without blocking user input.

## 5. VGA Output and Cursor Pitfalls
VGA text mode looks simple but is easy to misuse:
- **Unicode box characters rendered as garbage** because VGA only supports ASCII.
- **Hardware cursor stuck at top-left** because `ck_cursor_pos` was updated in software but never synced to the VGA hardware cursor registers.

Fixes included ASCII-only UI elements and explicit VGA cursor updates. This ensured the shell prompt and input cursor always appeared in the correct location.

## 6. Taskmon Display Bugs + Input Reliability
The task monitor introduced a new set of problems:
- Static bars because counters were monotonic and normalized to max
- Bars appeared empty due to block characters not supported by the VGA font
- Input was unreliable because only a single key was read per loop

These were solved by:
- Using ASCII `#` and `.` for bars
- Computing per-interval deltas instead of total counters
- Draining the keyboard buffer each loop
- Throttling refresh to a fixed frame rate

This made `taskmon` both readable and responsive, and also enabled the mini game to behave correctly.

## Debugging Lessons Learned
1. **Observe first, then change.** Small, reversible changes were the only way to avoid losing signal in a bare-metal environment.
2. **Instrumentation matters.** QEMU flags, serial output (where possible), and VGA status lines were essential tools.
3. **Order of initialization is critical.** Interrupts, PIC configuration, and IDT setup must happen in a strictly correct order.
4. **UI bugs are real kernel bugs.** VGA rendering and cursor handling are low-level device interactions, not just presentation.

These debugging episodes shaped the final architecture and reinforced the importance of disciplined testing and incremental integration. The devlog captures the exact investigations and fixes as part of the engineering narrative.

---

# Future Improvements
- Preemptive multitasking  
- User mode (Ring 3)  
- System call interface  
- Filesystem + disk driver  

---

# Conclusion
CheesecakeOS fulfills the assignment requirements and extends them with a live scheduler visualization and an interactive demo game. The project demonstrates deep understanding of x86 kernel fundamentals, interrupt handling, memory management, and multitasking concepts, presented in a polished and reviewer-friendly way.

---

# References
1. OSDev Wiki  
2. Intel(R) 64 and IA-32 Architectures Software Developer's Manual  
3. Multiboot2 Specification  
4. GNU GRUB Documentation  
5. QEMU Documentation  
6. The Little OS Book  
7. James Molloy's Kernel Development Tutorials
