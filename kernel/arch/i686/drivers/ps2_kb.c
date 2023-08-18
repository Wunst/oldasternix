#include <drivers/ps2_kb.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <stdio.h>

#include <drivers/kb.h>
#include <x86/interrupts.h>
#include <x86/pio.h>

#define IRQ1_KEYBOARD_DATA_READY 1

/* Keyboard controller ports */

#define PS2_KB_DATA 0x60
#define PS2_KB_CMD 0x64
#define PS2_KB_STATUS 0x64

/* Status byte flags */

#define PS2_KB_OUTPUT_BUFFER_STATUS (1 << 0)
#define PS2_KB_INPUT_BUFFER_STATUS (1 << 1)

/* Configuration byte flags */

#define PS2_KB_ENABLE_IRQ1 (1 << 0)
#define PS2_KB_ENABLE_IRQ2 (1 << 1)
#define PS2_KB_TRANSLATE (1 << 6)

/* Keyboard controller commands */

#define PS2_KB_GETCONFIG 0x20
#define PS2_KB_SETCONFIG 0x60
#define PS2_KB_DISABLE2 0xa7
#define PS2_KB_CONTROLLER_SELF_TEST 0xaa
#define PS2_KB_DISABLE1 0xad
#define PS2_KB_ENABLE1 0xae

/* Controller response */

#define PS2_KB_CONTROLLER_SELF_TEST_PASSED 0x55

/* Device commands (sent on the data port!) */

#define PS2_KB_SETSCANCODESET 0xf0
#define PS2_KB_RESET 0xff

/* Device response */

#define PS2_KB_SELF_TEST_PASSED 0xaa
#define PS2_KB_ACK 0xfa
#define PS2_KB_RESEND 0xfe

static uint8_t scancode_set;

static void wait_out(uint16_t port, uint8_t val)
{
    uint8_t status;
    do {
        status = inb(PS2_KB_STATUS);
    } while (status & PS2_KB_INPUT_BUFFER_STATUS);
    outb(port, val);
}

static uint8_t wait_in()
{
    uint8_t status;
    do {
        status = inb(PS2_KB_STATUS);
    } while (!(status & PS2_KB_OUTPUT_BUFFER_STATUS));
    return inb(PS2_KB_DATA);
}

static uint8_t send_dev_command1(uint8_t command)
{
    uint8_t status;
    do {
        wait_out(PS2_KB_DATA, command);
        status = wait_in();
    } while (status == PS2_KB_RESEND);
    return status;
}

static uint8_t send_dev_command2(uint8_t command, uint8_t arg)
{
    if (send_dev_command1(command) != PS2_KB_ACK) {
        printf("err: ps/2: first byte not ACKed: %02X %02X\n", command, arg);
    }
    return send_dev_command1(arg);
}

static void cfg_set(uint8_t flag)
{
    wait_out(PS2_KB_CMD, PS2_KB_GETCONFIG);
    uint8_t config = wait_in();
    config |= flag;
    wait_out(PS2_KB_CMD, PS2_KB_SETCONFIG);
    wait_out(PS2_KB_DATA, config);
}

static void cfg_clear(uint8_t flag)
{
    wait_out(PS2_KB_CMD, PS2_KB_GETCONFIG);
    uint8_t config = wait_in();
    config &= ~flag;
    wait_out(PS2_KB_CMD, PS2_KB_SETCONFIG);
    wait_out(PS2_KB_DATA, config);
}

static bool controller_setup()
{
    /* Self-test controller. */
    wait_out(PS2_KB_CMD, PS2_KB_CONTROLLER_SELF_TEST);
    if (wait_in() != PS2_KB_CONTROLLER_SELF_TEST_PASSED) {
        printf("err: ps/2: controller self test failed\n");
        return false;
    }

    /* Disable device port 2 (usually the mouse). */
    wait_out(PS2_KB_CMD, PS2_KB_DISABLE1);
    wait_out(PS2_KB_CMD, PS2_KB_DISABLE2);

    /* Disable translation. By default, the keyboard controller translates from
     * the keyboard's native scancode set (usually 2) to set 1 for
     * compatibility reasons. */
    cfg_clear(PS2_KB_ENABLE_IRQ1 | PS2_KB_ENABLE_IRQ2 | PS2_KB_TRANSLATE);

    /* Reset devices. */
    uint8_t response = send_dev_command1(PS2_KB_RESET);
    if (response != PS2_KB_ACK) {
        printf("warn: ps/2: unable to reset devices\n");
    } else if (wait_in() != PS2_KB_SELF_TEST_PASSED) {
        printf("err: ps/2: device self test failed\n");
        return false;
    }

    /* If supported, we want to use set 3, the most modern one and the only one
     * with the reasonable stance of reporting both pressing and releasing on
     * every key, including the Pause key. */
    response = send_dev_command2(PS2_KB_SETSCANCODESET, 3);
    if (response != PS2_KB_ACK) {
        printf("warn: ps/2: unable to set scancode set\n");
    }

    /* Check if it worked. If it didn't, we just have to use whatever the
     * keyboard supports, which SHOULD always be 2, but might be 1? */
    response = send_dev_command2(PS2_KB_SETSCANCODESET, 0);
    if (response != PS2_KB_ACK) {
        printf("err: ps/2: failed to determine scancode set\n");
        return false;
    }
    scancode_set = wait_in();
    if (scancode_set < 1 || scancode_set > 3) {
        printf("err: ps/2: unknown scancode set: %d\n", scancode_set);
        return false;
    }
    printf("info: ps/2: using scancode set %d\n", scancode_set);

    /* Re-enable IRQ1. */
    cfg_set(PS2_KB_ENABLE_IRQ1);

    /* Re-enable device port 1 (keyboard). */
    wait_out(PS2_KB_CMD, PS2_KB_ENABLE1);

    printf("info: ps/2: setup successful\n");
    return true;
}

INTERRUPT_HANDLER(ps2_key_pressed)()
{
    uint8_t scancode;
    scancode = inb(PS2_KB_DATA);

    printf("%02X\n", scancode);

    pic_eoi(IRQ1_KEYBOARD_DATA_READY);
}

void ps2_kb_driversetup()
{
    if (!controller_setup()) {
        printf("FATAL: ps/2: keyboard will not be available\n");
        return;
    }
    set_isr(IRQ_OFFSET + IRQ1_KEYBOARD_DATA_READY, int_ps2_key_pressed);
    pic_clear_mask(IRQ1_KEYBOARD_DATA_READY);
}

void ps2_kb_drivercleanup()
{
    pic_set_mask(IRQ1_KEYBOARD_DATA_READY);
    set_isr(IRQ_OFFSET + IRQ1_KEYBOARD_DATA_READY, NULL);
}
