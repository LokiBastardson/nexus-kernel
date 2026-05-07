#ifndef _KERNEL_PROCESS_SCHEDULER_H
#define _KERNEL_PROCESS_SCHEDULER_H

#include <types.h>
#include <process/process.h>
#include <cpu/idt.h>

#define SCHEDULER_QUANTUM 20

void scheduler_init(void);
void scheduler_add(process_t *proc);
void scheduler_remove(process_t *proc);
void scheduler_tick(registers_t *regs);
void scheduler_yield(void);
void scheduler_block_current(process_state_t reason);
void scheduler_unblock(process_t *proc);

#endif
