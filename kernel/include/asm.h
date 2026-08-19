// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// 包括一些汇编操作

#ifndef __ASM_H
#define __ASM_H

#include <stdint.h>

#define EFLAGS_IF 0x00000200

static inline void hlt()
{
    asm("hlt");
}

static inline void cli()
{
    asm("cli");
}

static inline void sti()
{
    asm("sti");
}

static inline void nop()
{
    asm("nop");
}

static inline void pause()
{
    asm("pause");
}

static inline void outb(uint16_t port, uint8_t data)
{
    asm("outb %%al, %%dx" ::"a"(data), "d"(port));
    nop();
    nop();
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm("inb %%dx, %%al" : "=a"(ret) : "d"(port));
    nop();
    nop();
    return ret;
}

static inline unsigned int read_eflags()
{
    unsigned int flags;
    asm("pushf\n\t"
        "popl %0"
        : "=r"(flags));
    return flags;
}

static inline void write_eflags(unsigned int flags)
{
    asm("pushl %0\n\t"
        "popf" ::"r"(flags));
}

static inline void set_if(unsigned int val)
{
    val ? sti() : cli();
}

#endif
