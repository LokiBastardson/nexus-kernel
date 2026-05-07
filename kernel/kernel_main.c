#include <types.h>
#include <boot/multiboot.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/io.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <memory/kheap.h>
#include <process/process.h>
#include <process/scheduler.h>
#include <fs/vfs.h>
#include <fs/ramfs.h>
#include <drivers/driver.h>
#include <drivers/timer.h>
#include <drivers/keyboard.h>
#include <syscall/syscall.h>
#include <lib/stdio.h>
#include <lib/string.h>

static void demo_process_a(void)
{
    for (;;)
        scheduler_yield();
}

static void demo_process_b(void)
{
    for (;;)
        scheduler_yield();
}

static void test_filesystem(void)
{
    vfs_node_t *root = vfs_get_root();
    if (!root) {
        kprintf("[TEST] ERROR: No root filesystem\n");
        return;
    }

    vfs_mkdir(root, "dev", 0755);
    vfs_mkdir(root, "tmp", 0777);
    vfs_mkdir(root, "etc", 0755);

    vfs_create(root, "hello.txt", VFS_FILE);
    vfs_node_t *hello = vfs_finddir(root, "hello.txt");
    if (hello) {
        const char *msg = "Hello from NexusKernel!\n";
        vfs_write(hello, 0, strlen(msg), (const uint8_t *)msg);

        uint8_t buf[64];
        memset(buf, 0, sizeof(buf));
        ssize_t nread = vfs_read(hello, 0, sizeof(buf) - 1, buf);
        if (nread > 0)
            kprintf("[TEST] Read from hello.txt: %s", (char *)buf);
    }

    kprintf("[TEST] Filesystem contents:\n");
    uint32_t i = 0;
    vfs_node_t *child;
    while ((child = vfs_readdir(root, i)) != NULL) {
        const char *type_str = (child->type & VFS_DIRECTORY) ? "DIR " : "FILE";
        kprintf("  %s  %s  (%u bytes)\n", type_str, child->name, child->size);
        i++;
    }
}

void kernel_main(uint32_t magic, multiboot_info_t *mbi)
{
    /* Initialize VGA for output */
    kprintf("========================================\n");
    kprintf("  NexusKernel v0.1.0\n");
    kprintf("  x86 Operating System Kernel\n");
    kprintf("========================================\n\n");

    /* Verify multiboot */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf("ERROR: Invalid multiboot magic: %x\n", magic);
        kprintf("Expected: %x\n", MULTIBOOT_BOOTLOADER_MAGIC);
        cli();
        for (;;)
            hlt();
    }
    kprintf("[BOOT] Multiboot verified (magic=%x)\n", magic);

    /* CPU descriptor tables */
    kprintf("[BOOT] Initializing GDT...\n");
    gdt_init();
    kprintf("[BOOT] Initializing IDT...\n");
    idt_init();

    /* Memory subsystem */
    kprintf("[BOOT] Initializing physical memory manager...\n");
    pmm_init(mbi);

    kprintf("[BOOT] Initializing kernel heap...\n");
    kheap_init();

    /* Driver framework */
    kprintf("[BOOT] Initializing driver manager...\n");
    driver_manager_init();

    /* Hardware drivers */
    kprintf("[BOOT] Initializing timer...\n");
    timer_init(TIMER_HZ);

    kprintf("[BOOT] Initializing keyboard...\n");
    keyboard_init();

    /* File system */
    kprintf("[BOOT] Initializing VFS...\n");
    vfs_init();

    vfs_node_t *ramfs_root = ramfs_init();
    if (ramfs_root)
        vfs_mount("/", "ramfs", ramfs_root);

    /* Process management */
    kprintf("[BOOT] Initializing process manager...\n");
    process_init();

    kprintf("[BOOT] Initializing scheduler...\n");
    scheduler_init();

    /* System calls */
    kprintf("[BOOT] Initializing system calls...\n");
    syscall_init();

    /* Run filesystem test */
    kprintf("\n[BOOT] Running filesystem self-test...\n");
    test_filesystem();

    /* Create demo processes */
    process_t *proc_a = process_create("worker_a", demo_process_a, 1);
    process_t *proc_b = process_create("worker_b", demo_process_b, 1);

    if (proc_a)
        scheduler_add(proc_a);
    if (proc_b)
        scheduler_add(proc_b);

    /* List registered drivers */
    kprintf("\n");
    driver_list_all();

    /* Enable interrupts */
    kprintf("\n[BOOT] Enabling interrupts...\n");
    sti();

    kprintf("[BOOT] NexusKernel is running!\n");
    kprintf("[BOOT] Memory: %u total frames, %u free\n",
            pmm_get_total_frames(), pmm_get_free_frames());
    kprintf("\n");

    /* Idle loop */
    for (;;)
        hlt();
}
