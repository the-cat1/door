// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __MM_H
#define __MM_H

#include <stdbool.h>
#include <stdint.h>

#define PAGE_P ((uint32_t)0x001)
#define PAGE_W ((uint32_t)0x002)
#define PAGE_U ((uint32_t)0x004)
#define PAGE_PWT ((uint32_t)0x008)
#define PAGE_PCD ((uint32_t)0x010)
#define PAGE_A ((uint32_t)0x020)
#define PAGE_D ((uint32_t)0x040)
#define PAGE_PAT ((uint32_t)0x080)
#define PAGE_G ((uint32_t)0x100)

void mm_page_init();
void *alloc_page_k(long count);
void free_page(void *vaddr, long count);
void mmap_page(long virtual_page, long phyical_page, uint32_t attr);
long alloc_phy_page();

#endif
