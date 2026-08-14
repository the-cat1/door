// kernel/main.c
// Enter point of kernel.

#include <stdbool.h>

#include "asm.h"
#include "boot.h"
#include "device/timer.h"
#include "irq.h"
#include "printk.h"
#include "video.h"

void kmain(void)
{
    init_video();
    printk("door kernel");

    init_irq();
    init_timer();
    sti();

    copy_multiboot_info();

    while (true)
        hlt();
}
