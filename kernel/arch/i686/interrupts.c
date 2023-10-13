#include <x86/interrupts.h>

#include <stdint.h>

#include <string.h>
#include <stdio.h>

#include <x86/pio.h>

#include "panic.h"

#define MS_CMD 0x20
#define MS_DATA 0x21
#define SL_CMD 0xa0
#define SL_DATA 0xa1

#define PIC_INIT 0x10
#define PIC_CASC 0x01
#define PIC_8086 0x01
#define PIC_EOI 0x20

/* defined in interrupts.s */
extern void load_enable_interrupts(void);
extern isr_stub *exc_isrs[32];

struct idt_entry {
    uint16_t offset_low;
    uint16_t segment;
    uint8_t _res32;
    unsigned type : 4;
    unsigned _res44 : 1;
    unsigned ring : 2;
    unsigned present : 1;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t size;
    struct idt_entry *base;
} __attribute__((packed));

struct idt_entry IDT[256] __attribute__((aligned(0x10)));
struct idt_ptr IDTR;

static void init_pic()
{
    /* Start init sequence (in cascade mode). */
    outb(MS_CMD, PIC_INIT | PIC_CASC);
    outb(SL_CMD, PIC_INIT | PIC_CASC);
    io_wait();

    /* Set IRQ offsets (interrupt vectors 0-31 are used for exceptions). */
    outb(MS_DATA, IRQ_OFFSET);
    outb(SL_DATA, IRQ_OFFSET + 8);
    io_wait();

    /* Tell master PIC the mask of slave PIC (bit 2 -> IRQ2 = cascade). */
    outb(MS_DATA, 0b00000100);

    /* Tell slave PIC which IRQ is the cascade IRQ. */
    outb(SL_DATA, 2);
    io_wait();

    /* Use 8086 mode (instead of 8080). */
    outb(MS_DATA, PIC_8086);
    outb(SL_DATA, PIC_8086);
    io_wait();

    /* Mask all interrupts other than cascade for now. */
    outb(MS_DATA, 0b11111011);
    outb(SL_DATA, 0b11111111);
    io_wait();
}

static void init_table()
{
    struct idt_entry base_entry;
    base_entry.offset_low = 0;
    base_entry.segment = 0x08; /* kernel code */
    base_entry._res32 = 0;
    base_entry.type = INTERRUPT;
    base_entry._res44 = 0;
    base_entry.ring = 0;
    base_entry.present = 0;
    base_entry.offset_high = 0;

    int i;
    for (i = 0; i < 256; i++) {
        memcpy(&IDT[i], &base_entry, sizeof(struct idt_entry));
    }

    IDTR.size = sizeof(IDT) - 1;
    IDTR.base = &IDT[0];
}

void setup_interrupts()
{
    init_pic();
    init_table();

    for (int i = 0; i < 32; i++)
        set_isr_type(i, exc_isrs[i], TRAP);

    load_enable_interrupts();
}

void set_isr(uint8_t int_no, isr_stub *stub)
{
    IDT[int_no].offset_low = (uint16_t)(uint32_t)stub;
    IDT[int_no].offset_high = (uint16_t)((uint32_t)stub >> 16);

    IDT[int_no].present = stub ? 1 : 0;
}

void set_isr_type(uint8_t int_no, isr_stub *stub, enum isr_type type)
{
    IDT[int_no].type = type;

    set_isr(int_no, stub);
}

void pic_set_mask(uint8_t irq)
{
    uint16_t port;
    uint8_t value;
    
    if (irq > 8) {
        port = SL_DATA;
        irq -= 8;
    } else {
        port = MS_DATA;
    }

    value = inb(port);
    value |= 1 << irq;
    outb(port, value);
    io_wait();
}

void pic_clear_mask(uint8_t irq)
{
    uint16_t port;
    uint8_t value;

    if (irq > 8) {
        port = SL_DATA;
        irq -= 8;
    } else {
        port = MS_DATA;
    }

    value = inb(port);
    value &= ~(1 << irq);
    outb(port, value);
    io_wait();
}

void pic_eoi(uint8_t irq)
{
    if (irq >= 8)
        outb(SL_CMD, PIC_EOI);
    
    outb(MS_CMD, PIC_EOI);
}

static const char *exceptions[32] = {
    "Division Error (#DE)",
    "Debug (#DB)",
    "<Non-Maskable Interrupt>",
    "Breakpoint (#BP)",
    "Overflow (#OF)",
    "Bound Range Exceeded (#BR)",
    "Invalid Opcode (#UD)",
    "Device Not Available (#NM)",
    "Double Fault (#DF)",
    "Coprocessor Segment Overrun",
    "Invalid TSS (#TS)",
    "Segment Not Present (#NP)",
    "Stack-Segment Fault (#SS)",
    "General Protection Fault (#GP)",
    "Page Fault (#PF)",
    "<Reserved>",
    "x87 Floating-Point Exception (#MF)",
    "Alignment Check (#AC)",
    "Machine Check (#MC)",
    "SIMD Floating-Point Exception (#XM)",
    "Virtualization Exception (#VE)",
    "Control Protection Exception (#CP)",
    "<Reserved>",
    "<Reserved>",
    "<Reserved>",
    "<Reserved>",
    "<Reserved>",
    "<Reserved>",
    "Hypervisor Injection Exception (#HV)",
    "VMM Communication Exception (#VC)",
    "Security Exception (#SX)",
    "<Reserved>",
};

void exception_code(uint32_t error, struct error_registers regs)
{
    const char *name = exceptions[error];
    panic("An exception occured: %s\n"
        "Error code: %08x\n"
        "Register dump:\n"
        "%%eax = %08x\t%%ebx = %08x\n"
        "%%ecx = %08x\t%%edx = %08x\n"
        "%%ebp = %08x\t%%esp = %08x\n"
        "%%esi = %08x\t%%edi = %08x\n"
        "%%eflags = %08x\n",
        name, regs.error_code, regs.eax, regs.ebx, regs.ecx, regs.edx,
        regs.ebp, regs.esp, regs.esi, regs.edi, regs.eflags);
}

void exception_no_code(uint32_t error, struct registers regs)
{
    const char *name = exceptions[error];
    panic("An exception occured: %s\n"
        "Register dump:\n"
        "%%eax = %08x\t%%ebx = %08x\n"
        "%%ecx = %08x\t%%edx = %08x\n"
        "%%ebp = %08x\t%%esp = %08x\n"
        "%%esi = %08x\t%%edi = %08x\n"
        "%%eflags = %08x\n",
        name, regs.eax, regs.ebx, regs.ecx, regs.edx, regs.ebp, regs.esp,
        regs.esi, regs.edi, regs.eflags);
}
