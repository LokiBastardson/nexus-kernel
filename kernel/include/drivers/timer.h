#ifndef _KERNEL_DRIVERS_TIMER_H
#define _KERNEL_DRIVERS_TIMER_H

#include <types.h>

#define PIT_FREQ      1193180
#define TIMER_HZ      100
#define PIT_CHANNEL0  0x40
#define PIT_CMD       0x43

void     timer_init(uint32_t frequency);
uint64_t timer_get_ticks(void);
void     timer_sleep(uint32_t ms);

#endif
