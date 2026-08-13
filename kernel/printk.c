// kernel/printk.c

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
    snprintf(buffer, sizeof(buffer), "[%05lu] ", ticks);
    print_string(buffer);

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    print_string(buffer);

    print_string("\n"); // 换行
}
