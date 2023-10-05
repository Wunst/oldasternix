    .section .multiboot, "ax"
    .set magic, 0xe85250d6
    .set architecture, 0
    .set checksum, -(magic + architecture + header_length)

    .long magic
    .long architecture
    .long header_length
    .long checksum

    .set module_align_tag, 6
    .set module_align_tag_length, 8
    .long module_align_tag
    .long module_align_tag_length

    .set end_tag, 0
    .set end_tag_length, 8
    .long end_tag
    .long end_tag_length

    .set header_length, . - .multiboot

    .set kernel_virtual_offset, 0xC0000000

    .global _start
_start:
    /* Disable interrupts until set up */
    cli

    /* TODO: Sanity check multiboot (magic value) */

    /* Load paging structures. */
    movl $initpd - kernel_virtual_offset, %edi
    movl %edi, %cr3

    /* Map the last page directory entry to itself. This will allow us to
     * access the page tables as pages starting at 0xffc00000 and the page
     * directory itself at 0xfffff000. */
    or $0x00000003, %edi /* Present + Read/Write */
    movl %edi, initpd - kernel_virtual_offset + (4 * 0x3ff)

    /* Build a page table. Map the kernel in the lower half (required since we
     * are currently executing from there) and again in the higher half, at
     * 0xc0000000 (where we want to jump to). */
    movl $initpt - kernel_virtual_offset, %edi
    or $0x00000003, %edi
    movl %edi, initpd - kernel_virtual_offset
    movl %edi, initpd - kernel_virtual_offset + (4 * 0x300)

    movl $initpt - kernel_virtual_offset, %edi
    movl $0x00000003, %eax
    movl $1024, %ecx
.fill_table:
    movl %eax, (%edi)
    add $4, %edi
    add $0x00001000, %eax
    loop .fill_table

    /* Enable paging. */
    movl %cr0, %eax
    or $1<<31, %eax
    movl %eax, %cr0

    /* Load the GDT and selectors. */
    lgdt gdt_ptr
    mov $k_data, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    /* Provide kernel call stack. The stack grows downwards on x86. */
    movl $stack_top, %esp
    movl %esp, %ebp

    /* Call into high level kernel.
     * Calling convention is a bit weird: SYSTEM V ABI specifies the stack
     * should be 16-byte aligned before using `call`, so we skip 12 bytes, then
     * push our 4-byte argument. Then the C function expects us to push return
     * address, but ljmp doesn't push any return address while lcall pushes an
     * additional 4 bytes for return segment, breaking any argument passing
     * entirely. So since we don't want to return anyways, we just skip another
     * 4 bytes as a "pseudo-return address", then use a normal ljmp. */
    subl $0x0c, %esp
    push %ebx
    subl $0x04, %esp
    ljmp $k_code, $hlinit

    /* unreachable */

    .section .text
    .global halt_loop
/*
 * void halt_loop(void);
 * Puts the system into an idle loop.
 */
halt_loop:
    hlt
    jmp halt_loop

    .section .data

    /* GDT (general descriptor table) contains segment descriptors.
     * Segmentation is a legacy memory protection technique which we don't use,
     * but the table is required to provide information such as privilege
     * levels for kernel and user contexts, so we just say all segments apply
     * to the whole address space (flat memory model). */
    .global k_code, k_data, u_code, u_data
gdt:
    .set GRAN_4K, 1<<7
    .set BITS32, 1<<6
    
    .set PRESENT, 1<<7
    .set NSYSTEM, 1<<4
    .set EXECUTABLE, 1<<3
    .set RW, 1<<1

    .set RING0, 0<<5
    .set RING3, 3<<5

    /* Null descriptor. */
    .long 0, 0
    
    /*
     * Kernel code.
     * Base = 0x00000000
     * Limit = 0xfffff (=4GiB at 4KiB granularity)
     * Access = Ring 0, r-x
     */
    .set k_code, . - gdt
    .word 0xffff, 0x0000
    .byte 0x00
    .byte PRESENT | NSYSTEM | RW | EXECUTABLE | RING0
    .byte GRAN_4K | BITS32 | 0xf
    .byte 0x00

    /*
     * Kernel data.
     * Access = Ring 0, rw-
     */
    .set k_data, . - gdt
    .word 0xffff, 0x0000
    .byte 0x00
    .byte PRESENT | NSYSTEM | RW | RING0
    .byte GRAN_4K | BITS32 | 0xf
    .byte 0x00

    /*
     * User code.
     * Access = Ring 3, r-x
     */
    .set u_code, . - gdt
    .word 0xffff, 0x0000
    .byte 0x00
    .byte PRESENT | NSYSTEM | RW | EXECUTABLE | RING3
    .byte GRAN_4K | BITS32 | 0xf
    .byte 0x00

    /*
     * User data.
     * Access = Ring 3, rw-
     */
    .set u_data, . - gdt
    .word 0xffff, 0x0000
    .byte 0x00
    .byte PRESENT | NSYSTEM | RW | RING3
    .byte GRAN_4K | BITS32 | 0xf
    .byte 0x00

gdt_ptr:
    .word . - gdt - 1
    .long gdt

    .section .bss
    .align 4096
    /* Initial paging structures. */
initpd:
    .skip 4096
initpt:
    .skip 4096

    .align 16
    /* Kernel stack. */
stack_bottom:
    .skip 8192 /* 8 KiB */
stack_top:
