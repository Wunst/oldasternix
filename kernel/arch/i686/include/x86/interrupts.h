/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

#define IRQ_OFFSET 0x20

#define INTERRUPT_HANDLER(name) extern isr_stub int_##name; void name

typedef void isr_stub(void);

struct registers {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx, edx, ecx, eax;

    uint32_t eip;
    uint16_t cs;
    uint16_t _padding;
    uint32_t eflags;
};

struct error_registers {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx, edx, ecx, eax;

    uint32_t error_code;

    uint32_t eip;
    uint16_t cs;
    uint16_t _padding;
    uint32_t eflags;
};

enum isr_type {
    INTERRUPT = 0xe,
    TRAP = 0xf,
};

void setup_interrupts(void);
void set_isr(uint8_t int_no, isr_stub *stub);
void set_isr_type(uint8_t int_no, isr_stub *stub, enum isr_type type);

void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);

void pic_eoi(uint8_t irq);

#endif
