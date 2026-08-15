// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include <stddef.h>

#include "lib/string.h"
#include "assert.h"
#include "bitmap.h"

/**
 * @brief 清除 bitmap
 *
 * @param map Bitmap
 *
 * @warning 需确 map->bits 和 map->size 正确
 */
void bitmap_clear(struct bitmap *map)
{
    memset(map->bits, 0, map->size);
}

/**
 * @brief 获取 bitmap的某一位
 *
 * @param map bitmap
 * @param index 要获取的下标
 * @return map 中 index 的值 (0/1)
 *
 * @note 需保证 index < map->size * 8
 */
int bitmap_get(struct bitmap *map, size_t index)
{
    size_t byte_idx = index / 8;
    assert(byte_idx < map->size);
    return map->bits[byte_idx] >> index % 8 & 1;
}

/**
 * @brief 设置bitmap的某一位
 *
 * @param map bitmap
 * @param index 要设置的下标
 * @param value 要设置的值
 *
 * @note 需保证 index < map->size * 8
 */
void bitmap_set(struct bitmap *map, size_t index, int value)
{
    size_t byte_idx = index / 8;
    assert(byte_idx < map->size);
    char mask = 1 << index % 8;
    if (value)
        map->bits[byte_idx] |= mask;
    else
        map->bits[byte_idx] &= ~mask;
}

/**
 * @brief 在 bitmap 中搜索一个空闲位，将这位置一
 *
 * @param map Bitmap
 * @return 这一位的下标
 *
 * @note 如果找不到空闲的位，返回 -1
 */
size_t bitmap_alloc(struct bitmap *map)
{
    for (size_t i = 0; i < map->size; i++) {
        unsigned char byte = map->bits[i];
        if (byte == 0xFF) // is it full?
            continue;

        // 搜索每一位
        for (int j = 0; i < 8; j++) {
            if (!(byte >> j & 1)) {
                // 找到空闲的一位，将其置 1
                byte |= 1 << j;
                map->bits[i] = byte;
                return i * 8 + j; // 返回下标
            }
        }
    }

    return -1; // 未找到
}
