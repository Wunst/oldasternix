#include <stdint.h>

#include <stdio.h>

#include <drivers/tty.h>
#include <drivers/ps2_kb.h>
#include <x86/interrupts.h>
#include <x86/mem.h>

/* defined in boot0.s */
extern void halt_loop(void);

void hlinit(uint32_t mbinfo_phys)
{
    mem_init();
    tty_init();

    setup_interrupts();

    ps2_kb_driversetup();

    printf("Hello, world!\n");

    halt_loop();
}
