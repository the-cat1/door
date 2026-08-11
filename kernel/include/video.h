// kernel/include/video.h

#ifndef __VIDEO_H
#define __VIDEO_H

#include <stddef.h>

#define VIDEO_ADDRESS ((uint8_t *)0xb8000) // 文本模式显存起始地址
#define VIDEO_DEF_ATTR 0x07             // 默认的文字属性
#define VIDEO_COLS 80                   // 列数
#define VIDEO_ROWS 25                   // 行数

#define VIDEO_IO_INDEX 0x03d4   // vga 显卡索引寄存器
#define VIDEO_IO_DATA 0x03d5    // vga 数据寄存器
#define VIDEO_IDX_CURPOS_H 0x0e // 光标位置高位
#define VIDEO_IDX_CURPOS_L 0x0f // 光标位置低位

void init_video();
void print_char(char c, char attr);
size_t print_string(const char *str);

#endif
