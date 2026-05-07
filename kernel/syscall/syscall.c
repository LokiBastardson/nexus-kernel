#include <types.h>
#include <syscall/syscall.h>
#include <cpu/idt.h>
#include <process/process.h>
#include <process/scheduler.h>
#include <fs/vfs.h>
#include <lib/stdio.h>

static int32_t sys_exit(registers_t *regs)
{
    int exit_code = (int)regs->ebx;
    process_t *current = process_get_current();
    if (current)
        process_terminate(current, exit_code);
    scheduler_yield();
    return 0;
}

static int32_t sys_read(registers_t *regs)
{
    int fd = (int)regs->ebx;
    uint8_t *buffer = (uint8_t *)regs->ecx;
    size_t size = (size_t)regs->edx;

    (void)fd;
    (void)buffer;
    (void)size;
    return -1;
}

static int32_t sys_write(registers_t *regs)
{
    int fd = (int)regs->ebx;
    const char *buffer = (const char *)regs->ecx;
    size_t size = (size_t)regs->edx;

    if (fd == 1 || fd == 2) {
        for (size_t i = 0; i < size; i++)
            kprintf("%c", buffer[i]);
        return (int32_t)size;
    }

    return -1;
}

static int32_t sys_open(registers_t *regs)
{
    const char *path = (const char *)regs->ebx;
    uint32_t flags = regs->ecx;

    vfs_node_t *node = vfs_resolve_path(path);
    if (!node)
        return -1;

    return vfs_open(node, flags);
}

static int32_t sys_close(registers_t *regs)
{
    (void)regs;
    return 0;
}

static int32_t sys_fork(registers_t *regs)
{
    (void)regs;
    return -1;
}

static int32_t sys_getpid(registers_t *regs)
{
    (void)regs;
    process_t *current = process_get_current();
    if (current)
        return current->pid;
    return -1;
}

static int32_t sys_yield(registers_t *regs)
{
    (void)regs;
    scheduler_yield();
    return 0;
}

static int32_t sys_sbrk(registers_t *regs)
{
    (void)regs;
    return -1;
}

static int32_t sys_mkdir(registers_t *regs)
{
    const char *path = (const char *)regs->ebx;
    mode_t mode = (mode_t)regs->ecx;

    vfs_node_t *root = vfs_get_root();
    if (!root)
        return -1;

    return vfs_mkdir(root, path, mode);
}

static int32_t sys_unlink(registers_t *regs)
{
    const char *path = (const char *)regs->ebx;

    vfs_node_t *root = vfs_get_root();
    if (!root)
        return -1;

    return vfs_unlink(root, path);
}

static int32_t sys_stat(registers_t *regs)
{
    (void)regs;
    return -1;
}

static syscall_fn_t syscall_table[NUM_SYSCALLS] = {
    [SYSCALL_EXIT]   = sys_exit,
    [SYSCALL_READ]   = sys_read,
    [SYSCALL_WRITE]  = sys_write,
    [SYSCALL_OPEN]   = sys_open,
    [SYSCALL_CLOSE]  = sys_close,
    [SYSCALL_FORK]   = sys_fork,
    [SYSCALL_GETPID] = sys_getpid,
    [SYSCALL_YIELD]  = sys_yield,
    [SYSCALL_SBRK]   = sys_sbrk,
    [SYSCALL_MKDIR]  = sys_mkdir,
    [SYSCALL_UNLINK] = sys_unlink,
    [SYSCALL_STAT]   = sys_stat,
};

static void syscall_handler(registers_t *regs)
{
    uint32_t num = regs->eax;

    if (num >= NUM_SYSCALLS) {
        regs->eax = (uint32_t)-1;
        return;
    }

    if (syscall_table[num])
        regs->eax = (uint32_t)syscall_table[num](regs);
    else
        regs->eax = (uint32_t)-1;
}

void syscall_init(void)
{
    isr_register_handler(128, syscall_handler);
    kprintf("[SYSCALL] System call interface initialized (%d calls)\n", NUM_SYSCALLS);
}
