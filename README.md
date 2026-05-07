# NexusKernel

A minimal x86 operating system kernel written in C from the ground up.

## Architecture

NexusKernel is a monolithic kernel targeting the x86 (i686) architecture with the following subsystems:

### Core Components

- **Boot** — Multiboot-compliant entry point (GRUB-compatible), GDT/IDT setup, assembly bootstrap
- **Memory Management** — Bitmap-based physical memory manager (PMM), page-table virtual memory manager (VMM), kernel heap allocator (`kmalloc`/`kfree`)
- **Process Management** — Process creation/termination, round-robin preemptive scheduler with context switching
- **File System** — Virtual File System (VFS) abstraction layer with pluggable filesystem support, built-in RAM filesystem (ramfs)
- **Driver Model** — Registration-based driver framework supporting character, block, network, display, input, and storage device types
- **System Calls** — `int 0x80` syscall interface with 12 system calls (exit, read, write, open, close, fork, getpid, yield, sbrk, mkdir, unlink, stat)
- **Interrupt Handling** — Full ISR/IRQ infrastructure, PIC remapping, PIT timer (100 Hz), PS/2 keyboard driver

### Project Structure

```
kernel/
├── boot/           # Multiboot entry, ISR/IRQ stubs (assembly)
├── cpu/            # GDT, IDT, PIC setup
├── memory/         # Physical memory, virtual memory, kernel heap
├── process/        # Process management, round-robin scheduler
├── fs/             # VFS layer, RAM filesystem
├── drivers/        # Driver framework, PIT timer, PS/2 keyboard
├── syscall/        # System call dispatcher and handlers
├── lib/            # Freestanding libc (string, printf)
├── include/        # All kernel headers
└── kernel_main.c   # Kernel entry point
```

## Building

### Prerequisites

- `i686-linux-gnu-gcc` cross-compiler toolchain
- `make`
- `qemu-system-i386` (for testing)
- `grub-mkrescue` and `xorriso` (for ISO generation)

On Ubuntu/Debian:

```bash
sudo apt-get install gcc-i686-linux-gnu binutils-i686-linux-gnu make qemu-system-x86 grub-pc-bin xorriso
```

### Build Commands

```bash
make            # Build the kernel binary
make iso        # Build a bootable ISO image
make run        # Build ISO and launch in QEMU
make run-kernel # Launch kernel directly in QEMU (no ISO)
make clean      # Remove build artifacts
```

## Design Decisions

- **Freestanding C** — No standard library dependency; all libc functions implemented from scratch
- **Bitmap PMM** — Simple and efficient for tracking physical page frames
- **Round-robin scheduling** — Fair, preemptive scheduling with configurable time quantum
- **VFS abstraction** — Clean separation between filesystem interface and implementation
- **Driver registration model** — Drivers self-register with type, ops, and lifecycle callbacks
- **`-Wall -Wextra -Werror`** — Strict compilation with all warnings treated as errors

## License

MIT
