#include <types.h>
#include <lib/stdio.h>
#include <lib/string.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000
#define VGA_COLOR  0x07

static uint16_t *vga_buffer = (uint16_t *)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;

static void vga_scroll(void)
{
    if (cursor_y >= VGA_HEIGHT) {
        memmove(vga_buffer, vga_buffer + VGA_WIDTH,
                VGA_WIDTH * (VGA_HEIGHT - 1) * sizeof(uint16_t));
        for (int i = 0; i < VGA_WIDTH; i++)
            vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + i] =
                (uint16_t)(' ' | (VGA_COLOR << 8));
        cursor_y = VGA_HEIGHT - 1;
    }
}

static void vga_putchar(char c)
{
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 8) & ~7;
    } else if (c == '\b') {
        if (cursor_x > 0)
            cursor_x--;
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] =
            (uint16_t)(c | (VGA_COLOR << 8));
        cursor_x++;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    vga_scroll();
}

static void print_str(const char *s)
{
    while (*s)
        vga_putchar(*s++);
}

static void print_int(int32_t value)
{
    char buf[12];
    int i = 0;
    bool negative = false;

    if (value < 0) {
        negative = true;
        value = -value;
    }

    if (value == 0) {
        vga_putchar('0');
        return;
    }

    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    if (negative)
        vga_putchar('-');

    while (i > 0)
        vga_putchar(buf[--i]);
}

static void print_uint(uint32_t value)
{
    char buf[12];
    int i = 0;

    if (value == 0) {
        vga_putchar('0');
        return;
    }

    while (value > 0) {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0)
        vga_putchar(buf[--i]);
}

static void print_hex(uint32_t value)
{
    const char hex[] = "0123456789abcdef";
    char buf[9];
    int i = 0;

    if (value == 0) {
        print_str("0x0");
        return;
    }

    while (value > 0) {
        buf[i++] = hex[value & 0xF];
        value >>= 4;
    }

    print_str("0x");
    while (i > 0)
        vga_putchar(buf[--i]);
}

void kprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
            case 'd':
            case 'i':
                print_int(va_arg(args, int32_t));
                break;
            case 'u':
                print_uint(va_arg(args, uint32_t));
                break;
            case 'x':
            case 'X':
            case 'p':
                print_hex(va_arg(args, uint32_t));
                break;
            case 's':
                print_str(va_arg(args, const char *));
                break;
            case 'c':
                vga_putchar((char)va_arg(args, int));
                break;
            case '%':
                vga_putchar('%');
                break;
            default:
                vga_putchar('%');
                vga_putchar(*fmt);
                break;
            }
        } else {
            vga_putchar(*fmt);
        }
        fmt++;
    }

    va_end(args);
}
