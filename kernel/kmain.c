// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// Enter point of kernel.

#include <stdbool.h>

#include "asm.h"
#include "boot.h"
#include "device/timer.h"
#include "irq.h"
#include "mm.h"
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
    mm_page_init();

    while (true)
        hlt();
}
