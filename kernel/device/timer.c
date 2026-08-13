// kernel/device/timer.c

#include <stdint.h>

#include "asm.h"
#include "printk.h"
#include "irq.h"
#define __DEVICE_TIMER_C
#include "device/timer.h"

#define INPUT_FREQ ((unsigned long)1193180)
#define PIT_IO_CTR 0x43
#define PIT_IO_COUNTER0 0x40
#define COUNTER0_VALUE (INPUT_FREQ / TICKS_FREQ)

static void time_handler(struct irq_frame *frame)
{
    (void)frame;
    ticks++;
    send_eoi();
}

void init_timer()
{
    outb(PIT_IO_CTR, 0x34);
    outb(PIT_IO_COUNTER0, (uint8_t)(COUNTER0_VALUE & 0xFF));
    outb(PIT_IO_COUNTER0, (uint8_t)(COUNTER0_VALUE >> 8));
    register_irq(0x20, time_handler);
    printk(
        "init timer ok, in freq %lu / counter value %lu = ticks freq %lu", INPUT_FREQ, COUNTER0_VALUE, TICKS_FREQ
    );
}
