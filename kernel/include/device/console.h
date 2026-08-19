// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __VIDEO_H
#define __VIDEO_H

#include <stddef.h>

void console_init();
void console_putc(int c);
size_t console_print(const char *str);

#endif
