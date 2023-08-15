ENTRY(_start)

SECTIONS
{
    . = 1M;

    .multiboot ALIGN(4K) :
    {
        KEEP(*(.multiboot))
    }

    PROVIDE(__kernel_virtual_offset = 0xC0000000);
    . += __kernel_virtual_offset;

    PROVIDE(__kernel_start = .);

    .text ALIGN(4K) : AT(ADDR(.text) - __kernel_virtual_offset)
    {
        *(.text*)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - __kernel_virtual_offset)
    {
        *(.data*)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - __kernel_virtual_offset)
    {
        *(.bss*)
        *(COMMON)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - __kernel_virtual_offset)
    {
        *(.rodata*)
    }

    PROVIDE(__kernel_end = .);

    /DISCARD/ :
    {
        *(.comment* .note*)
        *(.eh_frame*)
    }
}
