// kernel/include/panic.h

#ifndef __PANIC_H
#define __PANIC_H

#define panic(...) _panic(__FILE__, __func__, __LINE__, __VA_ARGS__)

void _panic(const char *file, const char *func, int line, const char *fmt, ...);

#endif
