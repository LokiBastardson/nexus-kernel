#include <types.h>
#include <drivers/timer.h>
#include <drivers/driver.h>
#include <cpu/idt.h>
#include <cpu/io.h>
#include <process/scheduler.h>
#include <lib/stdio.h>

static uint64_t timer_ticks = 0;
static uint32_t timer_frequency = 0;

static void timer_callback(registers_t *regs)
{
    timer_ticks++;
    scheduler_tick(regs);
}

static int timer_driver_init(driver_t *drv)
{
    (void)drv;
    return 0;
}

static void timer_driver_cleanup(driver_t *drv)
{
    (void)drv;
}

static driver_ops_t timer_driver_ops = {
    .init    = timer_driver_init,
    .cleanup = timer_driver_cleanup,
    .read    = NULL,
    .write   = NULL,
    .ioctl   = NULL,
};

static driver_t timer_driver = {
    .name  = "pit_timer",
    .type  = DRIVER_TYPE_OTHER,
    .state = DRIVER_STATE_UNLOADED,
    .major = 0,
    .minor = 0,
    .ops   = &timer_driver_ops,
    .private_data = NULL,
    .next  = NULL,
};

void timer_init(uint32_t frequency)
{
    timer_frequency = frequency;
    timer_ticks = 0;

    uint32_t divisor = PIT_FREQ / frequency;

    outb(PIT_CMD, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    isr_register_handler(32, timer_callback);

    driver_register(&timer_driver);
    driver_load("pit_timer");

    kprintf("[TIMER] PIT initialized at %u Hz\n", frequency);
}

uint64_t timer_get_ticks(void)
{
    return timer_ticks;
}

void timer_sleep(uint32_t ms)
{
    uint64_t target = timer_ticks + (ms * timer_frequency / 1000);
    while (timer_ticks < target)
        hlt();
}
