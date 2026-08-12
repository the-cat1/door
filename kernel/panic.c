// kernel/panic.c

#include <stdarg.h>
#include <stdbool.h>

#include "lib/sprintf.h"
#include "asm.h"
#include "printk.h"
#include "panic.h"

void _panic(const char *file, int line, const char *func, const char *fmt, ...)
{
    va_list args;
    char buf[1024];

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    printk("kernel panic: %s", buf);
    printk("  at %s:%d (%s)", file, line, func);

    cli(); // 关闭中断
    while (true)
        hlt();
}
