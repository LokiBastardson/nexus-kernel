#include <types.h>
#include <drivers/keyboard.h>
#include <drivers/driver.h>
#include <cpu/idt.h>
#include <cpu/io.h>
#include <lib/stdio.h>

static char    kb_buffer[KB_BUFFER_SIZE];
static uint32_t kb_read_idx;
static uint32_t kb_write_idx;
static uint32_t kb_count;

static const char scancode_to_ascii[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

static void keyboard_callback(registers_t *regs)
{
    (void)regs;

    uint8_t scancode = inb(KB_DATA_PORT);

    /* Only handle key press (bit 7 clear = key down) */
    if (scancode & 0x80)
        return;

    char c = scancode_to_ascii[scancode];
    if (c && kb_count < KB_BUFFER_SIZE) {
        kb_buffer[kb_write_idx] = c;
        kb_write_idx = (kb_write_idx + 1) % KB_BUFFER_SIZE;
        kb_count++;
    }
}

static int keyboard_driver_init(driver_t *drv)
{
    (void)drv;
    return 0;
}

static void keyboard_driver_cleanup(driver_t *drv)
{
    (void)drv;
}

static ssize_t keyboard_driver_read(driver_t *drv, void *buffer, size_t size)
{
    (void)drv;
    char *buf = (char *)buffer;
    size_t read = 0;

    while (read < size && kb_count > 0) {
        buf[read++] = kb_buffer[kb_read_idx];
        kb_read_idx = (kb_read_idx + 1) % KB_BUFFER_SIZE;
        kb_count--;
    }

    return (ssize_t)read;
}

static driver_ops_t keyboard_driver_ops = {
    .init    = keyboard_driver_init,
    .cleanup = keyboard_driver_cleanup,
    .read    = keyboard_driver_read,
    .write   = NULL,
    .ioctl   = NULL,
};

static driver_t keyboard_driver = {
    .name  = "ps2_keyboard",
    .type  = DRIVER_TYPE_INPUT,
    .state = DRIVER_STATE_UNLOADED,
    .major = 1,
    .minor = 0,
    .ops   = &keyboard_driver_ops,
    .private_data = NULL,
    .next  = NULL,
};

void keyboard_init(void)
{
    kb_read_idx = 0;
    kb_write_idx = 0;
    kb_count = 0;

    isr_register_handler(33, keyboard_callback);

    driver_register(&keyboard_driver);
    driver_load("ps2_keyboard");

    kprintf("[KB] PS/2 keyboard driver initialized\n");
}

char keyboard_getchar(void)
{
    while (kb_count == 0)
        hlt();

    char c = kb_buffer[kb_read_idx];
    kb_read_idx = (kb_read_idx + 1) % KB_BUFFER_SIZE;
    kb_count--;
    return c;
}

bool keyboard_has_input(void)
{
    return kb_count > 0;
}
