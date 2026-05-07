#include <types.h>
#include <cpu/idt.h>
#include <cpu/io.h>
#include <lib/string.h>
#include <lib/stdio.h>

#define IDT_ENTRIES 256
#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1

static idt_entry_t idt_entries[IDT_ENTRIES];
static idt_ptr_t   idt_ptr;
static isr_handler_t isr_handlers[IDT_ENTRIES];

static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel       = sel;
    idt_entries[num].always0   = 0;
    idt_entries[num].flags     = flags | 0x60;
}

static void pic_remap(void)
{
    outb(PIC1_CMD,  0x11); io_wait();
    outb(PIC2_CMD,  0x11); io_wait();
    outb(PIC1_DATA, 0x20); io_wait();
    outb(PIC2_DATA, 0x28); io_wait();
    outb(PIC1_DATA, 0x04); io_wait();
    outb(PIC2_DATA, 0x02); io_wait();
    outb(PIC1_DATA, 0x01); io_wait();
    outb(PIC2_DATA, 0x01); io_wait();
    outb(PIC1_DATA, 0x00); io_wait();
    outb(PIC2_DATA, 0x00); io_wait();
}

void idt_init(void)
{
    idt_ptr.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entry_t) * IDT_ENTRIES);
    memset(&isr_handlers, 0, sizeof(isr_handlers));

    pic_remap();

    idt_set_gate(0,  (uint32_t)isr0,  KERNEL_CS, 0x8E);
    idt_set_gate(1,  (uint32_t)isr1,  KERNEL_CS, 0x8E);
    idt_set_gate(2,  (uint32_t)isr2,  KERNEL_CS, 0x8E);
    idt_set_gate(3,  (uint32_t)isr3,  KERNEL_CS, 0x8E);
    idt_set_gate(4,  (uint32_t)isr4,  KERNEL_CS, 0x8E);
    idt_set_gate(5,  (uint32_t)isr5,  KERNEL_CS, 0x8E);
    idt_set_gate(6,  (uint32_t)isr6,  KERNEL_CS, 0x8E);
    idt_set_gate(7,  (uint32_t)isr7,  KERNEL_CS, 0x8E);
    idt_set_gate(8,  (uint32_t)isr8,  KERNEL_CS, 0x8E);
    idt_set_gate(9,  (uint32_t)isr9,  KERNEL_CS, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, KERNEL_CS, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, KERNEL_CS, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, KERNEL_CS, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, KERNEL_CS, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, KERNEL_CS, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, KERNEL_CS, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, KERNEL_CS, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, KERNEL_CS, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, KERNEL_CS, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, KERNEL_CS, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, KERNEL_CS, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, KERNEL_CS, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, KERNEL_CS, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, KERNEL_CS, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, KERNEL_CS, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, KERNEL_CS, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, KERNEL_CS, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, KERNEL_CS, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, KERNEL_CS, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, KERNEL_CS, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, KERNEL_CS, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, KERNEL_CS, 0x8E);

    idt_set_gate(32, (uint32_t)irq0,  KERNEL_CS, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  KERNEL_CS, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  KERNEL_CS, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  KERNEL_CS, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  KERNEL_CS, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  KERNEL_CS, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  KERNEL_CS, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  KERNEL_CS, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  KERNEL_CS, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  KERNEL_CS, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, KERNEL_CS, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, KERNEL_CS, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, KERNEL_CS, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, KERNEL_CS, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, KERNEL_CS, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, KERNEL_CS, 0x8E);

    idt_set_gate(128, (uint32_t)isr128, KERNEL_CS, 0xEE);

    idt_flush((uint32_t)&idt_ptr);
}

void isr_register_handler(uint8_t n, isr_handler_t handler)
{
    isr_handlers[n] = handler;
}

void isr_handler(registers_t *regs)
{
    if (isr_handlers[regs->int_no]) {
        isr_handlers[regs->int_no](regs);
    } else if (regs->int_no < 32) {
        kprintf("KERNEL PANIC: %s (int %d, err %x)\n",
                exception_messages[regs->int_no],
                regs->int_no, regs->err_code);
        kprintf("  EIP=%x CS=%x EFLAGS=%x\n", regs->eip, regs->cs, regs->eflags);
        kprintf("  EAX=%x EBX=%x ECX=%x EDX=%x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
        kprintf("  ESP=%x EBP=%x ESI=%x EDI=%x\n", regs->esp, regs->ebp, regs->esi, regs->edi);
        cli();
        for (;;)
            hlt();
    }
}

void irq_handler(registers_t *regs)
{
    if (regs->int_no >= 40)
        outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);

    if (isr_handlers[regs->int_no])
        isr_handlers[regs->int_no](regs);
}
