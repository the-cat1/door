// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// Enter point of kernel.

#include <stdbool.h>
#include <stdint.h>
#include "lib/string.h"

#include "asm.h"
#include "boot.h"
#include "device/console.h"
#include "device/timer.h"
#include "gdt.h"
#include "irq.h"
#include "mm.h"
#include "printk.h"
#include "task.h"

void task_switch_to(struct task_struct *);

int task_1(void *arg)
{
    printk("hello from task 1! %s", arg);
    while (true) {
        hlt();
    }
}

void utask_func()
{
    while (true)
        ;
}

struct task_struct *task1;
struct task_struct *task2;

void kmain(void)
{
    console_init();
    printk_init();
    printk("door kernel");

    gdt_init();
    init_irq();
    init_timer();
    copy_multiboot_info();
    mm_page_init();
    init_task();

    task1 = ktask_create("task 1", 20, task_1, "arg1");
    printk("task %p", task1);
    task_run(task1);

    mmap_page(1000, alloc_phy_page(), PAGE_P | PAGE_U | PAGE_W);
    memcpy((void *)(1000 * 4096), utask_func, 4096);
    task2 = utask_create("usertask2", 20, 1000 * 4096, 1001 * 4096);
    task_run(task2);

    // 打开中断
    // 当发生时钟中断时，就会切换到其他任务
    sti();

    while (true)
        hlt();
}
