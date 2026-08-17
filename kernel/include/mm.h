// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __MM_H
#define __MM_H

#include <stdbool.h>
#include <stdint.h>

void mm_page_init();
void *mm_page_alloc_k(long count);
void mm_page_free(void *vaddr, long count);

#endif
