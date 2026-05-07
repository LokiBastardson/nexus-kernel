CC      = i686-linux-gnu-gcc
AS      = i686-linux-gnu-as
LD      = i686-linux-gnu-gcc
OBJCOPY = i686-linux-gnu-objcopy

CFLAGS  = -std=gnu11 -ffreestanding -O2 -Wall -Wextra -Werror \
          -fno-exceptions -fno-stack-protector -nostdlib \
          -Ikernel/include
ASFLAGS =
LDFLAGS = -ffreestanding -O2 -nostdlib -no-pie -lgcc -T linker.ld

KERNEL  = nexus-kernel.bin
ISO     = nexus-kernel.iso

C_SOURCES = \
    kernel/lib/string.c \
    kernel/lib/stdio.c \
    kernel/cpu/gdt.c \
    kernel/cpu/idt.c \
    kernel/memory/pmm.c \
    kernel/memory/vmm.c \
    kernel/memory/kheap.c \
    kernel/process/process.c \
    kernel/process/scheduler.c \
    kernel/fs/vfs.c \
    kernel/fs/ramfs.c \
    kernel/drivers/driver.c \
    kernel/drivers/timer.c \
    kernel/drivers/keyboard.c \
    kernel/syscall/syscall.c \
    kernel/kernel_main.c

ASM_SOURCES = \
    kernel/boot/boot.s \
    kernel/boot/isr.s

C_OBJECTS   = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.s=.o)
OBJECTS     = $(ASM_OBJECTS) $(C_OBJECTS)

.PHONY: all clean iso run

all: $(KERNEL)

$(KERNEL): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

iso: $(KERNEL)
	mkdir -p isodir/boot/grub
	cp $(KERNEL) isodir/boot/nexus-kernel.bin
	echo 'menuentry "NexusKernel" {' > isodir/boot/grub/grub.cfg
	echo '    multiboot /boot/nexus-kernel.bin' >> isodir/boot/grub/grub.cfg
	echo '}' >> isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) isodir 2>/dev/null

run: iso
	qemu-system-i386 -cdrom $(ISO) -m 128M -serial stdio

run-kernel: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -m 128M -serial stdio

clean:
	rm -f $(OBJECTS) $(KERNEL) $(ISO)
	rm -rf isodir

info:
	@echo "Sources: $(C_SOURCES)"
	@echo "Objects: $(OBJECTS)"
