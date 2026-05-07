#ifndef _KERNEL_PROCESS_PROCESS_H
#define _KERNEL_PROCESS_PROCESS_H

#include <types.h>
#include <memory/vmm.h>

#define PROCESS_NAME_MAX 64
#define MAX_PROCESSES    256
#define KERNEL_STACK_SIZE 8192

typedef enum {
    PROCESS_CREATED,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp, esp;
    uint32_t eip, eflags;
    uint32_t cs, ds, es, fs, gs, ss;
    uint32_t cr3;
} cpu_context_t;

typedef struct process {
    pid_t              pid;
    pid_t              parent_pid;
    char               name[PROCESS_NAME_MAX];
    process_state_t    state;
    int                priority;
    int                exit_code;
    cpu_context_t      context;
    page_directory_t  *page_directory;
    uint32_t           kernel_stack;
    uint32_t           user_stack;
    struct process    *next;
} process_t;

void       process_init(void);
process_t *process_create(const char *name, void (*entry)(void), int priority);
void       process_terminate(process_t *proc, int exit_code);
process_t *process_get_current(void);
process_t *process_get_by_pid(pid_t pid);
pid_t      process_get_next_pid(void);

#endif
