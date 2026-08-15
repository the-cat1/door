// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __LIB_SPRINTF_H
#define __LIB_SPRINTF_H

#include <stdarg.h>
#include <stddef.h>

int vsnprintf(char *s, size_t size, const char *fmt, va_list args);
int vsprintf(char *s, const char *fmt, va_list args);
int snprintf(char *s, size_t size, const char *fmt, ...);
int sprintf(char *s, const char *fmt, ...);

#endif
