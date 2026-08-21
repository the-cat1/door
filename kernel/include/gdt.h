// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_DPL_KERNEL 0
#define GDT_DPL_USER 3

struct tss {
    uint16_t back_selector, __back_selector;
    uint32_t sp0;
    uint16_t ss0, __ss0;
    uint32_t sp1;
    uint16_t ss1, __ss1;
    uint32_t sp2;
    uint16_t ss2, __ss2;
    uint32_t cr3;
    uint32_t ip;
    uint32_t flags;
    uint32_t ax, cx, dx, bx, sp, bp, si, di;
    uint16_t es, __es;
    uint16_t cs, __cs;
    uint16_t ss, __ss;
    uint16_t ds, __ds;
    uint16_t fs, __fs;
    uint16_t gs, __gs;
    uint16_t ldt_selector, __ldt_selector;
    uint16_t trace;
    uint16_t io_map;
} __attribute__((__packed__));

#ifdef __GDT_C
#define extern
#endif

extern struct tss tss;

extern uint16_t selector_kernel_code;
extern uint16_t selector_kernel_data;
extern uint16_t selector_user_code;
extern uint16_t selector_user_data;
extern uint16_t selector_tss;

#ifdef __GDT_C
#undef extern
#endif

void gdt_init();

#endif
