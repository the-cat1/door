// kernel/printk.c

#include <stdarg.h>
#include <stdint.h>

#include "lib/sprintf.h"
#include "printk.h"

void scroll_screen();
void write_char(char c, char attribute);
void set_char_attribute(int row, int col, uint16_t char_attribute);
uint16_t get_char_attribute(int row, int col);

int cursor_col = 0;
int cursor_row = 0;

void printk(const char *format, ...)
{
    va_list args;
    char buffer[1024];
    va_start(args, format);

    vsnprintf(buffer, sizeof(buffer), format, args);

    for (char *buffer_ptr = buffer; *buffer_ptr != '\0'; buffer_ptr++)
    {
        write_char(*buffer_ptr, ATTRIBUTE);
    }

    va_end(args);
}

uint16_t get_char_attribute(int row, int col)
{
    return (uint16_t)*(VIDEO_MEMORY + (row * COLS + col) * 2) << 8 |
           *(VIDEO_MEMORY + (row * COLS + col) * 2 + 1);
}

void set_char_attribute(int row, int col, uint16_t char_attribute)
{
    *(VIDEO_MEMORY + (row * COLS + col) * 2) = (char)(char_attribute >> 8);
    *(VIDEO_MEMORY + (row * COLS + col) * 2 + 1) = (char)(char_attribute & 0xFF);
}

void write_char(char c, char attribute)
{
    if (c == '\r')
    {
        cursor_col = 0;
        return;
    }
    else if (c == '\n')
    {
        cursor_col = 0;
        cursor_row++;
    }
    else
    {
        set_char_attribute(cursor_row, cursor_col, (uint16_t)c << 8 | attribute);
        cursor_col++;
    }

    if (cursor_col >= COLS)
    {
        cursor_col = 0;
        cursor_row++;
    }

    if (cursor_row >= ROWS)
    {
        scroll_screen();
        cursor_row = ROWS - 1;
    }
}

void scroll_screen()
{
    // 向上移动 0 ~ (ROWS - 1) 行
    for (int row = 0; row < ROWS - 1; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            set_char_attribute(row, col, get_char_attribute(row + 1, col));
        }
    }

    // 清空最后一行
    for (int col = 0; col < COLS; col++)
    {
        set_char_attribute(ROWS - 1, col, 0);
    }
}
