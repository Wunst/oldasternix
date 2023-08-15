    .section .text
    .global load_enable_interrupts

/*
 * void load_enable_interrupts(void);
 */
load_enable_interrupts:
    lidt IDTR
    sti
    ret

    .macro isr_stub handler
    .global int_\handler
int_\handler:
    pushal
    call \handler
    popal
    iret
    .endm

    isr_stub ps2_key_pressed
