#ifndef _KERNEL_SYSCALL_SYSCALL_H
#define _KERNEL_SYSCALL_SYSCALL_H

#include <types.h>
#include <cpu/idt.h>

#define SYSCALL_EXIT     0
#define SYSCALL_READ     1
#define SYSCALL_WRITE    2
#define SYSCALL_OPEN     3
#define SYSCALL_CLOSE    4
#define SYSCALL_FORK     5
#define SYSCALL_GETPID   6
#define SYSCALL_YIELD    7
#define SYSCALL_SBRK     8
#define SYSCALL_MKDIR    9
#define SYSCALL_UNLINK   10
#define SYSCALL_STAT     11

#define NUM_SYSCALLS     12

typedef int32_t (*syscall_fn_t)(registers_t *);

void syscall_init(void);

#endif
