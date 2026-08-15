/*
 * SPDX-FileCopyrightText: 2026 Zhan Zixuan
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#include <stddef.h>

#include "lib/math.h"
#include "asm.h"
#include "video.h"

int cursor_pos;

static void set_cursor_pos()
{
    outb(VIDEO_IO_INDEX, VIDEO_IDX_CURPOS_L);
    outb(VIDEO_IO_DATA, cursor_pos & 0xff);
    outb(VIDEO_IO_INDEX, VIDEO_IDX_CURPOS_H);
    outb(VIDEO_IO_DATA, cursor_pos >> 8);
}

static void get_cursor_pos()
{
    outb(VIDEO_IO_INDEX, VIDEO_IDX_CURPOS_L);
    uint8_t low = inb(VIDEO_IO_DATA);
    outb(VIDEO_IO_INDEX, VIDEO_IDX_CURPOS_H);
    uint8_t high = inb(VIDEO_IO_DATA);

    cursor_pos = low | high << 8;
}

static void set_char(int pos, uint8_t c, uint8_t attr)
{
    VIDEO_ADDRESS[pos * 2 + 0] = c;
    VIDEO_ADDRESS[pos * 2 + 1] = attr;
}

// 滚屏
static void scroll_up()
{
    /* 整体移动 */
    for (int i = VIDEO_COLS; i < VIDEO_ROWS * VIDEO_COLS; i++)
        set_char(i - VIDEO_COLS, VIDEO_ADDRESS[i * 2], VIDEO_ADDRESS[i * 2 + 1]);

    /* 清空最后一行 */
    for (int i = 0; i < VIDEO_COLS; i++)
        set_char((VIDEO_ROWS - 1) * VIDEO_COLS + i, 0, VIDEO_DEF_ATTR);

    cursor_pos -= VIDEO_COLS;
}

// '\r'
static void carriage_return()
{
    cursor_pos /= VIDEO_COLS;
    cursor_pos *= VIDEO_COLS;
}

// '\n'
static void feed_line()
{
    carriage_return();
    cursor_pos += VIDEO_COLS;
}

/**
 * @brief 初始化屏幕
 *
 */
void init_video()
{
    get_cursor_pos();
    print_string("\n\n");
}

/**
 * @brief 输出一个字符, 支持控制符号
 *
 * @param c 要输出的字符
 * @param attr 属性，可以使用默认的 `VIDEO_DEF_ATTR`
 */
void print_char(char c, char attr)
{
    switch (c) {
    case '\n':
        feed_line();
        break;

    case '\r':
        carriage_return();
        break;

    default:
        set_char(cursor_pos, c, attr);
        cursor_pos++;
        break;
    }

    if (cursor_pos < VIDEO_COLS * VIDEO_ROWS) {
        set_cursor_pos();
        return; // needn't scroll
    }

    scroll_up();
    set_cursor_pos();
}

/**
 * @brief 打印一个字符串
 *
 * @param str 要打印的字符串
 * @return size_t 输出的字符串总数
 */
size_t print_string(const char *str)
{
    size_t count = 0;
    while (*str) {
        print_char(*str, VIDEO_DEF_ATTR);
        count++;
        str++;
    }

    return count;
}
