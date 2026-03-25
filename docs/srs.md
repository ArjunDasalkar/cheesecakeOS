# Software Requirements Specification (SRS)

## 1. Functional Requirements

The system shall:

- Boot using a bootloader (GRUB)
- Display output to screen using VGA buffer
- Handle hardware interrupts
- Accept keyboard input
- Provide a basic command-line interface
- Manage memory allocation
- Support simple task execution

---

## 2. Non-Functional Requirements

### Performance
- Fast boot time in emulator
- Efficient memory usage

### Reliability
- Stable execution without crashes
- Proper interrupt handling

### Maintainability
- Modular code structure
- Clear documentation

### Portability
- Designed for x86 architecture
- Runs on QEMU emulator

---

## 3. Constraints

- Written in C and Assembly
- No external OS libraries
- Limited hardware abstraction