// kernel/printk.c

#include <stdarg.h>
#include <stdint.h>

#include "lib/sprintf.h"
#include "video.h"
#include "printk.h"

void printk(const char *format, ...)
{
    va_list args;
    char buffer[1024];
    va_start(args, format);

    vsnprintf(buffer, sizeof(buffer), format, args);

    print_string(buffer);

    print_string("\n"); // 换行

    va_end(args);
}
