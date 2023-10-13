    .section .text
    .global load_enable_interrupts

/*
 * void load_enable_interrupts(void);
 */
load_enable_interrupts:
    lidt IDTR
    sti
    ret

    .macro isr_exception_code n
int_exception_\n:
    pushal
    pushl $\n
    call exception_code
    add $4, %esp
    popal
    add $4, %esp
    iret
    .endm

    .macro isr_exception_no_code n
int_exception_\n:
    pushal
    pushl $\n
    call exception_no_code
    add $4, %esp
    popal
    iret
    .endm

    .macro isr_stub handler
    .global int_\handler
int_\handler:
    pushal
    call \handler
    popal
    iret
    .endm

    isr_exception_no_code 0
    isr_exception_no_code 1
    isr_exception_no_code 2
    isr_exception_no_code 3
    isr_exception_no_code 4
    isr_exception_no_code 5
    isr_exception_no_code 6
    isr_exception_no_code 7
    isr_exception_code 8
    isr_exception_no_code 9
    isr_exception_code 10
    isr_exception_code 11
    isr_exception_code 12
    isr_exception_code 13
    isr_exception_code 14
    isr_exception_no_code 15
    isr_exception_no_code 16
    isr_exception_code 17
    isr_exception_no_code 18
    isr_exception_no_code 19
    isr_exception_no_code 20
    isr_exception_code 21
    isr_exception_no_code 22
    isr_exception_no_code 23
    isr_exception_no_code 24
    isr_exception_no_code 25
    isr_exception_no_code 26
    isr_exception_no_code 27
    isr_exception_no_code 28
    isr_exception_code 29
    isr_exception_code 30
    isr_exception_no_code 31

    isr_stub ps2_key_pressed

    .section .data
    .global exc_isrs
exc_isrs:
    .altmacro
    .macro tableent i
    .long int_exception_\i
    .endm

    .set i, 0
    .rept 32
    tableent %i
    .set i, i + 1
    .endr
