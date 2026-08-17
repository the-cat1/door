// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// 管理一页（4k）的分配和释放

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bitmap.h"
#include "boot.h"
#include "lib/math.h"
#include "mm.h"
#include "panic.h"
#include "printk.h"

#define SEG_COUNT 16

#define PAGE_P ((uint32_t)0x001)
#define PAGE_W ((uint32_t)0x002)
#define PAGE_U ((uint32_t)0x004)
#define PAGE_PWT ((uint32_t)0x008)
#define PAGE_PCD ((uint32_t)0x010)
#define PAGE_A ((uint32_t)0x020)
#define PAGE_D ((uint32_t)0x040)
#define PAGE_PAT ((uint32_t)0x080)
#define PAGE_G ((uint32_t)0x100)

#define PTES ((uint32_t *)0xffc00000)
#define PDE ((uint32_t *)0xfffff000)

#define up_to_page(a) ((unsigned long)up_to((uintptr_t)(a), 4096) / 4096)
#define down_to_page(a) ((unsigned long)down_to((uintptr_t)(a), 4096) / 4096)
#define sub_v_offset(a) ((uintptr_t)(a) - (uintptr_t)(V_OFFSET))

struct mm_seg {
    unsigned long base; // in page
    unsigned long length; // in pages
    struct bitmap map;
};

static struct mm_seg segs[SEG_COUNT];
static unsigned long total_pages;

static void set_page(unsigned long page, uint32_t value)
{
    uint32_t pde_pa = PDE[page / 1024];
    if (pde_pa == 0)
        panic("pde is zero");

    PTES[page] = value;
}

static uint32_t get_page(unsigned long page)
{
    uint32_t pde_val = PDE[page / 1024];
    if (pde_val == 0)
        return -1;

    return PTES[page];
}

static void set_page_phy(unsigned long vpage, unsigned long ppage, uint32_t attr)
{
    uint32_t value = ppage * 4096;
    value |= attr;
    set_page(vpage, value);
}

static void set_kernel_seg(void *start, void *end, uint32_t attr)
{
    unsigned long s_page = down_to_page(start);
    unsigned long e_page = up_to_page(end);

    for (; s_page < e_page; s_page++) {
        set_page_phy(s_page, s_page - down_to_page(V_OFFSET), attr);
    }
}

static void set_kernel_pages()
{
    set_kernel_seg(__text_start, __text_end, PAGE_P);
    set_kernel_seg(__rodata_start, __rodata_end, PAGE_P);
    set_kernel_seg(__data_start, __data_end, PAGE_P | PAGE_W);
    set_kernel_seg(__bss_start, __bss_end, PAGE_P | PAGE_W);

    // 将 __kernel_end 之后的页表项置 0
    // see boot/boot.asm
    uintptr_t page = up_to_page(__kernel_end);
    while (page < down_to_page(V_OFFSET) + 1024) {
        set_page(page, 0);
        page++;
    }
}

static void free_vpage(unsigned long vpage)
{
    uint32_t val = get_page(vpage);
    if (val == (uint32_t)-1)
        return;

    unsigned long ppage = val / 4096;
    for (int i = 0; i < SEG_COUNT; i++) {
        struct mm_seg seg = segs[i];
        if (seg.length == 0)
            continue;

        if (ppage < seg.base || ppage >= seg.base + seg.length)
            continue;

        bitmap_set(&seg.map, ppage - seg.base, 0);
        set_page(vpage, 0);
        return;
    }

    // no found
    printk("free_vpage: wrong vpage %lu", vpage);
}

static int page_alloc_phy(size_t ppages[], struct mm_seg **res_seg, unsigned long count)
{
    for (int j = 0; j < SEG_COUNT; j++) {
        struct mm_seg *seg = &segs[j];
        if (seg->length == 0)
            continue;

        size_t i = 0;
        while (true) {
            size_t ppage = bitmap_alloc(&seg->map);
            if (ppage == (size_t)-1)
                break;

            ppages[i++] = ppage;
            if (i >= count) {
                *res_seg = seg;
                return 0;
            }
        }

        // reset ppages
        for (; i > 0; i--)
            bitmap_set(&seg->map, ppages[i - 1], 0);
    }

    return -1; // not enough space
}

static int create_new_ptek()
{
    unsigned long i = up_to_page(__kernel_end) / 1024;
    for (; i < 1024; i++) {
        if (PDE[i] != 0)
            continue;

        size_t ppages[1];
        struct mm_seg *seg;
        if (page_alloc_phy(ppages, &seg, 1))
            return -1;

        unsigned long ppage = ppages[0] + seg->base;
        PDE[i] = ppage * 4096 | PAGE_P | PAGE_W;
        return 0;
    }

    printk("kernel memory may be full");
    return -1;
}


// 查找一块空闲的虚拟内存
static unsigned long mm_page_scan_kv(unsigned long count)
{
    unsigned long page = up_to_page(__kernel_end);
    while (true) {
        uint32_t val = get_page(page);
        if (val == (uint32_t)-1) {
            if (create_new_ptek())
                return -1;

            page = up_to_page(__kernel_end);
            continue;
        }

        if (val == 0) {
            unsigned long start_page = page;
            for (; page < start_page + count - 1; page++)
                if (get_page(page) != 0)
                    goto go_on;
            return start_page;
        }
    go_on:
        page++;
    }
}

void mm_page_init()
{
    set_kernel_pages();

    int j = 0;
    for (int i = 0; i < min(mmaps_u_count, SEG_COUNT); i++) {
        struct multiboot_mmap mmap = mmaps_u[i];
        if (mmap.base > UINTPTR_MAX - 4096)
            continue;
        unsigned long base = max(up_to_page(mmap.base), up_to_page(sub_v_offset(__kernel_end)));

        unsigned long length;
        if (mmap.base + mmap.length > UINTPTR_MAX)
            length = down_to_page(UINTPTR_MAX - mmap.base - 4096);
        else
            length = down_to_page(mmap.length);
        if (length <= 0)
            continue;

        size_t map_len = down_to(length, 8) / 8; // in bytes
        unsigned long map_pg_count = up_to_page(map_len);
        if (map_pg_count >= length || map_pg_count <= 0)
            continue;

        unsigned long seg_length = down_to(length - map_pg_count, 8); // 方便 bitmap 管理
        unsigned long seg_base = base + map_pg_count;
        map_len = seg_length / 8; // 根据新的长度重新设置

        unsigned long map_start_pg = mm_page_scan_kv(map_pg_count);
        if (map_start_pg == (unsigned long)-1)
            panic("cannot find virtual memory for bitmap of memory segment %d", i);
        for (unsigned long i = 0; i < map_pg_count; i++)
            set_page_phy(map_start_pg + i, base + i, PAGE_P | PAGE_W);

        segs[j] = (struct mm_seg){.base = seg_base, .length = seg_length};
        segs[j].map = (struct bitmap){.size = map_len, .bits = (unsigned char *)(map_start_pg * 4096)};
        bitmap_clear(&segs[j].map);
        j++;

        total_pages += length;

        if (j >= SEG_COUNT)
            break;
    }

    if (j <= 0)
        panic("no usable memory");

    printk("initialized memory pages");
    printk("  total %lu pages, %lu MiB", total_pages, total_pages * 4096 / 1024 / 1024);
}

void *mm_page_alloc_k(unsigned long count)
{
    size_t ppages[count];
    unsigned long vpage_start = mm_page_scan_kv(count);
    if (vpage_start == (unsigned long)-1)
        return NULL;

    struct mm_seg *seg;
    if (page_alloc_phy(ppages, &seg, count))
        return NULL;

    for (size_t k = 0; k < count; k++)
        set_page_phy(vpage_start + k, ppages[k] + seg->base, PAGE_P); //| PAGE_W);
    return (void *)(vpage_start * 4096);
}

void mm_page_free(void *vaddr, unsigned long count)
{
    unsigned long vpage = down_to_page(vaddr);
    for (unsigned long i = 0; i < count; i++)
        free_vpage(vpage + i);
}
