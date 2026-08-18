// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// Enter point of kernel.

#include <stdbool.h>
#include <stdint.h>

#include "asm.h"
#include "boot.h"
#include "device/timer.h"
#include "irq.h"
#include "mm.h"
#include "printk.h"
#include "task.h"
#include "video.h"

void task_switch_to(struct task_struct *);

void task_1()
{
    while (true) {
        printk("hello from task 1!");
    }
}

void task_2()
{
    while (true) {
        // hlt();
        printk("hello from task 2!");
    }
}

void kmain(void)
{
    init_video();
    printk("door kernel");

    init_irq();
    init_timer();
    copy_multiboot_info();
    mm_page_init();
    init_task();

    struct task_struct *task1 = task_create("LOL task", 20, (uintptr_t)task_1);
    printk("task %p", task1);
    task_run(task1);

    struct task_struct *task2 = task_create("LOL task 2", 20, (uintptr_t)task_2);
    printk("task %p", task2);
    task_run(task2);

    sti();

    while (true)
        hlt();
}
