#include <types.h>
#include <process/process.h>
#include <memory/kheap.h>
#include <memory/vmm.h>
#include <lib/string.h>
#include <lib/stdio.h>

static process_t *process_table[MAX_PROCESSES];
static process_t *current_process;
static pid_t      next_pid = 1;

void process_init(void)
{
    memset(process_table, 0, sizeof(process_table));
    current_process = NULL;

    process_t *idle = process_create("idle", NULL, 0);
    if (idle) {
        idle->pid = 0;
        idle->state = PROCESS_RUNNING;
        idle->page_directory = vmm_get_kernel_directory();
        current_process = idle;
    }

    kprintf("[PROC] Process manager initialized\n");
}

process_t *process_create(const char *name, void (*entry)(void), int priority)
{
    pid_t pid = process_get_next_pid();
    if (pid < 0)
        return NULL;

    process_t *proc = (process_t *)kmalloc(sizeof(process_t));
    if (!proc)
        return NULL;

    memset(proc, 0, sizeof(process_t));
    proc->pid = pid;
    proc->parent_pid = current_process ? current_process->pid : 0;
    strncpy(proc->name, name, PROCESS_NAME_MAX - 1);
    proc->state = PROCESS_CREATED;
    proc->priority = priority;
    proc->next = NULL;

    proc->page_directory = vmm_create_address_space();
    if (!proc->page_directory) {
        kfree(proc);
        return NULL;
    }

    uint32_t *kstack = (uint32_t *)kmalloc(KERNEL_STACK_SIZE);
    if (!kstack) {
        vmm_destroy_address_space(proc->page_directory);
        kfree(proc);
        return NULL;
    }

    proc->kernel_stack = (uint32_t)kstack + KERNEL_STACK_SIZE;

    if (entry) {
        memset(&proc->context, 0, sizeof(cpu_context_t));
        proc->context.eip = (uint32_t)entry;
        proc->context.cs = KERNEL_CS;
        proc->context.ds = KERNEL_DS;
        proc->context.es = KERNEL_DS;
        proc->context.fs = KERNEL_DS;
        proc->context.gs = KERNEL_DS;
        proc->context.ss = KERNEL_DS;
        proc->context.esp = proc->kernel_stack;
        proc->context.ebp = proc->kernel_stack;
        proc->context.eflags = 0x202;
        proc->context.cr3 = proc->page_directory->physical_addr;
    }

    process_table[pid] = proc;
    proc->state = PROCESS_READY;

    kprintf("[PROC] Created process '%s' (PID %d)\n", name, pid);
    return proc;
}

void process_terminate(process_t *proc, int exit_code)
{
    if (!proc)
        return;

    proc->state = PROCESS_TERMINATED;
    proc->exit_code = exit_code;

    if (proc->pid > 0 && proc->pid < MAX_PROCESSES)
        process_table[proc->pid] = NULL;

    kprintf("[PROC] Process '%s' (PID %d) terminated with code %d\n",
            proc->name, proc->pid, exit_code);
}

process_t *process_get_current(void)
{
    return current_process;
}

process_t *process_get_by_pid(pid_t pid)
{
    if (pid < 0 || pid >= MAX_PROCESSES)
        return NULL;
    return process_table[pid];
}

pid_t process_get_next_pid(void)
{
    for (pid_t i = next_pid; i < MAX_PROCESSES; i++) {
        if (!process_table[i]) {
            next_pid = i + 1;
            return i;
        }
    }
    for (pid_t i = 1; i < next_pid; i++) {
        if (!process_table[i]) {
            next_pid = i + 1;
            return i;
        }
    }
    return -1;
}
