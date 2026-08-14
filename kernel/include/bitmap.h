// kernel/include/bitmap.h

#ifndef __BITMAP_H
#define __BITMAP_H

#include <stddef.h>

struct bitmap {
    size_t size; // in bytes
    unsigned char *bits;
};

void bitmap_clear(struct bitmap *map);
int bitmap_get(struct bitmap *map, size_t index);
void bitmap_set(struct bitmap *map, size_t index, int value);
size_t bitmap_alloc(struct bitmap *map);

#endif
