# CheesecakeOS - Architecture Diagrams

## Boot Sequence

```mermaid
sequenceDiagram
    participant BIOS
    participant GRUB
    participant Kernel
    participant Init as Kernel Init
    participant Shell

    BIOS->>GRUB: Load bootloader from ISO
    GRUB->>Kernel: Load kernel at 0x100000
    Kernel->>Init: ck_start -> ck_main
    Init->>Init: IDT, PIC, IRQs, timer, keyboard
    Init->>Init: pmem, paging, heap, scheduler
    Init->>Shell: Start shell loop
```

## Interrupt Flow (IRQ 0-1)

```mermaid
flowchart TD
    A[Hardware IRQ] --> B[PIC remap 32-47]
    B --> C[IRQ stub in irq.asm]
    C --> D[ck_irq_handler]
    D --> E{IRQ source}
    E -->|Timer IRQ0| F[ck_timer_irq_handler]
    E -->|Keyboard IRQ1| G[ck_keyboard_irq_handler]
    F --> H[Increment tick counter]
    G --> I[Buffer scancode]
    H --> J[Send EOI]
    I --> J
    J --> K[IRET]
```

## Memory Layout (Simplified)

```mermaid
flowchart TB
    A[0x00000000 - 0x0009FFFF
Lower memory, BIOS data] --> B[0x000A0000 - 0x000FFFFF
BIOS ROM]
    B --> C[0x000B8000
VGA text buffer]
    C --> D[0x00100000
Kernel load address]
    D --> E[Paging: identity map
first 256MB]
```

## Scheduler Loop (Cooperative)

```mermaid
flowchart TD
    A[Shell main loop] --> B[ck_scheduler_run_tasks]
    B --> C[Task 0 step]
    B --> D[Task 1 step]
    B --> E[Task 2 step]
    C --> F[Return]
    D --> F
    E --> F
    F --> G[Draw status]
    G --> H[Read keyboard]
    H --> A
```

## Task Monitor Data Flow

```mermaid
flowchart TD
    A[ck_scheduler_run_tasks] --> B[Task counters updated]
    B --> C[Compute per-frame deltas]
    C --> D[CPU bars + percent]
    D --> E[Draw task rows]
    E --> F[Draw mini game]
    F --> G[Poll keyboard A/D/Q]
    G --> A
```

## Task Monitor Layout

```mermaid
graph TB
    A[Row 0-1: Title + divider] --> B[Row 2: Controls]
    B --> C[Rows 3-11: Mini game]
    C --> D[Rows 12-15: Task rows + CPU bars]
    D --> E[Row 20: Summary stats]
    E --> F[Row 23: Quit hint]
```
