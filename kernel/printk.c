// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// 简单的 printk 实现

#include <stdarg.h>
#include <stdint.h>

#include "device/timer.h"
#include "lib/sprintf.h"
#include "video.h"
#include "task.h"
#include "printk.h"

static struct lock lock;
static int inited;

void printk(const char *format, ...)
{
    if (!inited)
        lock_init(&lock), inited++;

    va_list args;
    char buffer[1024];
    lock_acquire(&lock);
    // 打印头
    snprintf(buffer, sizeof(buffer), "[%03u.%03u] ", ticks / TICKS_FREQ, ticks % TICKS_FREQ * 1000 / TICKS_FREQ);
    print_string(buffer);

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    print_string(buffer);

    print_string("\n"); // 换行
    lock_release(&lock);
}
