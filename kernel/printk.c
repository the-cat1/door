// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// 简单的 printk 实现

#include <stdarg.h>
#include <stdint.h>

#include "device/timer.h"
#include "lib/sprintf.h"
#include "video.h"
#include "printk.h"

void printk(const char *format, ...)
{
    va_list args;
    char buffer[1024];

    // 打印头
    snprintf(buffer, sizeof(buffer), "[%03u.%03u] ", ticks / TICKS_FREQ, ticks % TICKS_FREQ * 1000 / TICKS_FREQ);
    print_string(buffer);

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    print_string(buffer);

    print_string("\n"); // 换行
}
