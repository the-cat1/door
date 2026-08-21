// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include <stdint.h>
#include "lib/string.h"
#include "panic.h"

#define __GDT_C
#include "gdt.h"

#define GDT_AVL 0x01 // available
#define GDT_L 0x02 // limit
#define GDT_DB 0x04 // double word / quad word
#define GDT_G 0x08 // granularity

#define AC_X 0x08 // execute
#define AC_CE 0x04 // conforming / expand down
#define AC_RW 0x02 // read / write

#define AC_S 0x10 // segment
#define AC_P 0x80 // present

#define AC_TSS 0x09 // 386+ task state segment

struct seg_desc {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    unsigned limit_high : 4;
    unsigned flags : 4;
    uint8_t base_high;
} __attribute__((packed));

#define DESC_MAX 6

static struct seg_desc gdt[DESC_MAX];
static int desc_count = 0;

static int install_entry(uintptr_t base, uintptr_t limit, uint8_t access, uint8_t flags)
{
    if (desc_count >= DESC_MAX)
        return -1;

    gdt[desc_count++] = (struct seg_desc){
        .base_low = base & 0xffff,
        .base_mid = base >> 16 & 0xff,
        .base_high = base >> 24,
        .limit_low = limit & 0xffff,
        .limit_high = limit >> 16,
        .access = access,
        .flags = flags
    };
    return desc_count - 1;
}

static uint16_t create_entry(uintptr_t base, uintptr_t limit, uint8_t dpl, uint8_t access, uint8_t flags)
{
    int index = install_entry(base, limit, access | (dpl << 3), flags);
    if (index < 0)
        panic("cannot install gdt entry.");

    uint16_t selector = index << 3;
    selector |= dpl;
    return selector;
}

void gdt_init()
{
    memset(&tss, 0, sizeof(tss));

    create_entry(0, 0, 0, 0, 0);
    selector_kernel_code = create_entry(0, 0xfffff, GDT_DPL_KERNEL, AC_X | AC_RW | AC_P | AC_S, GDT_DB | GDT_G);
    selector_kernel_data = create_entry(0, 0xfffff, GDT_DPL_KERNEL, AC_RW | AC_P | AC_S, GDT_DB | GDT_G);
    selector_user_code = create_entry(0, 0xfffff, GDT_DPL_USER, AC_X | AC_RW | AC_P, GDT_DB | GDT_G);
    selector_user_data = create_entry(0, 0xfffff, GDT_DPL_USER, AC_RW | AC_P | AC_S, GDT_DB | GDT_G);
    selector_tss = create_entry((uintptr_t)&tss, sizeof(tss), GDT_DPL_KERNEL, AC_P | AC_TSS, GDT_DB);

    uint64_t gdt_size = desc_count * 8 - 1;
    uint64_t gdtr_val = (uint64_t)(uintptr_t)gdt << 16 | gdt_size;
    asm("lgdt %0" ::"m"(gdtr_val));
    asm("movw %0, %%ss\n\t"
        "movw %0, %%ds\n\t"
        "movw %0, %%es\n\t"
        "movw %0, %%fs\n\t"
        "movw %0, %%gs\n\t" ::"r"(selector_kernel_data));
}
