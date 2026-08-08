// kernel/include/panic.h

#ifndef __PANIC_H
#define __PANIC_H

#define panic(...) _panic(__FILE__, __LINE__, __func__, __VA_ARGS__)

void _panic(const char *file, int line, const char *func, const char *fmt, ...);

#endif
