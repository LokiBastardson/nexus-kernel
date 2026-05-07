#include <types.h>
#include <process/scheduler.h>
#include <process/process.h>
#include <cpu/gdt.h>
#include <cpu/io.h>
#include <lib/stdio.h>

static process_t *ready_queue_head;
static process_t *ready_queue_tail;
static uint32_t   ticks_remaining;
static bool       scheduler_enabled = false;

void scheduler_init(void)
{
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    ticks_remaining = SCHEDULER_QUANTUM;
    scheduler_enabled = true;

    kprintf("[SCHED] Round-robin scheduler initialized (quantum=%d ticks)\n",
            SCHEDULER_QUANTUM);
}

void scheduler_add(process_t *proc)
{
    if (!proc)
        return;

    proc->next = NULL;

    if (!ready_queue_head) {
        ready_queue_head = proc;
        ready_queue_tail = proc;
    } else {
        ready_queue_tail->next = proc;
        ready_queue_tail = proc;
    }
}

void scheduler_remove(process_t *proc)
{
    if (!proc)
        return;

    if (ready_queue_head == proc) {
        ready_queue_head = proc->next;
        if (!ready_queue_head)
            ready_queue_tail = NULL;
        proc->next = NULL;
        return;
    }

    process_t *prev = ready_queue_head;
    while (prev && prev->next != proc)
        prev = prev->next;

    if (prev) {
        prev->next = proc->next;
        if (ready_queue_tail == proc)
            ready_queue_tail = prev;
        proc->next = NULL;
    }
}

static void context_switch(process_t *prev, process_t *next)
{
    if (prev == next)
        return;

    tss_set_kernel_stack(next->kernel_stack);

    if (next->page_directory)
        vmm_switch_directory(next->page_directory);

    /*
     * Save/restore register context using inline assembly.
     * The actual context switch saves ESP/EBP/EIP so we can
     * resume the previous process later.
     */
    __asm__ volatile(
        "pushl %%ebp\n"
        "movl %%esp, %0\n"
        "movl %2, %%esp\n"
        "movl $1f, %1\n"
        "pushl %3\n"
        "ret\n"
        "1:\n"
        "popl %%ebp\n"
        : "=m"(prev->context.esp), "=m"(prev->context.eip)
        : "m"(next->context.esp), "m"(next->context.eip)
        : "memory"
    );
}

void scheduler_tick(registers_t *regs)
{
    (void)regs;

    if (!scheduler_enabled)
        return;

    if (ticks_remaining > 0) {
        ticks_remaining--;
        return;
    }

    ticks_remaining = SCHEDULER_QUANTUM;

    process_t *current = process_get_current();
    if (!current)
        return;

    if (!ready_queue_head)
        return;

    process_t *next = ready_queue_head;

    if (next == current)
        return;

    ready_queue_head = next->next;
    if (!ready_queue_head)
        ready_queue_tail = NULL;

    if (current->state == PROCESS_RUNNING) {
        current->state = PROCESS_READY;
        scheduler_add(current);
    }

    next->state = PROCESS_RUNNING;
    context_switch(current, next);
}

void scheduler_yield(void)
{
    process_t *current = process_get_current();
    if (!current || !ready_queue_head)
        return;

    ticks_remaining = SCHEDULER_QUANTUM;

    process_t *next = ready_queue_head;
    ready_queue_head = next->next;
    if (!ready_queue_head)
        ready_queue_tail = NULL;

    if (current->state == PROCESS_RUNNING) {
        current->state = PROCESS_READY;
        scheduler_add(current);
    }

    next->state = PROCESS_RUNNING;
    context_switch(current, next);
}

void scheduler_block_current(process_state_t reason)
{
    process_t *current = process_get_current();
    if (!current)
        return;

    current->state = reason;
    scheduler_yield();
}

void scheduler_unblock(process_t *proc)
{
    if (!proc)
        return;

    proc->state = PROCESS_READY;
    scheduler_add(proc);
}
