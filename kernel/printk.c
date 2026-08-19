// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// 简单的 printk 实现

#include <stdarg.h>
#include <stdint.h>

#include "device/console.h"
#include "device/timer.h"
#include "lib/sprintf.h"
#include "task.h"
#include "printk.h"

static struct lock printk_lock;

void printk_init()
{
    lock_init(&printk_lock);
}

void printk(const char *format, ...)
{
    va_list args;
    char buffer[1024];
    lock_acquire(&printk_lock);

    // 打印头
    snprintf(buffer, sizeof(buffer), "[%03u.%03u] ", ticks / TICKS_FREQ, ticks % TICKS_FREQ * 1000 / TICKS_FREQ);
    console_print(buffer);

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    console_print(buffer);

    console_print("\n"); // 换行
    lock_release(&printk_lock);
}
