// kernel/include/mm.h

#ifndef __MM_H
#define __MM_H

#include <stdbool.h>
#include <stdint.h>

void mm_page_init();
void *mm_page_alloc_k(unsigned long count);
void mm_page_free(void *vaddr, unsigned long count);

#endif
