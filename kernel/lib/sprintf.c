// kernel/lib/sprintf.c

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include "lib/ctype.h"
#include "lib/sprintf.h"
#include "lib/math.h"

#define FLAG_LEFT 0x0001
#define FLAG_SPACE 0x0002
#define FLAG_PLUS 0x0004
#define FLAG_HASH 0x0008
#define FLAG_ZERO 0x0010

enum LengthModifier
{
    LENGTH_NONE,     // int
    LENGTH_HH,       // char
    LENGTH_H,        // short int
    LENGTH_L,        // long int
    LENGTH_LL,       // long long int
    LENGTH_J,        // intmax_t
    LENGTH_Z,        // size_t
    LENGTH_T,        // ptrdiff_t
    LENGTH_L_CAPITAL // long double
};

struct convert_args
{
    char *buffer;
    size_t buf_size;
    enum LengthModifier length;
    int width;
    int precision;
    int flag;
    va_list *args;
};

static void write_wrong_format(char **ptr, char *end)
{
    static const char *wrong_format = "[WRONG FORMAT]";
    const char *p = wrong_format;

    while (*p && *ptr < end)
    {
        **ptr = *p;
        (*ptr)++;
        p++;
    }
}

// only postive numbers are supported
static int parse_str_to_int(const char **str, int *value)
{
    char buf[12]; // 最大 2,147,483,647，10 个字符
    int i = 0;
    int base = 1;

    while (isdigit(**str))
    {
        buf[i] = **str;
        if (i++ >= 12)
        {
            return -1; // 数字太长
        }
        (*str)++;
    }

    if (i <= 0)
        return -1; // 没有数字

    i--; // buf[i] 为最后一个数字
    *value = 0;
    while (i >= 0)
    {
        *value += base * (buf[i] - '0');
        base *= 10;
        i--;
    }
    return 0;
}

/**
 * 读取不同大小的有符号整数
 *
 * @param length 长度修饰符
 * @param is_negative 输出参数，指示读取的整数是否为负数
 * @param args 可变参数列表
 * @return 返回读取的整数值
 */
static uintmax_t get_va_arg_int(enum LengthModifier length, bool *is_negative, va_list *args)
{
    intmax_t value;
    switch (length)
    {
    case LENGTH_HH: // 对于 char 和 short，实际 va_list 里是 int 类型
    case LENGTH_H:
    case LENGTH_NONE:
        value = va_arg(*args, int);
        break;

    case LENGTH_L:
        value = va_arg(*args, long);
        break;

    case LENGTH_LL:
        value = va_arg(*args, long long);
        break;

    case LENGTH_J:
        value = va_arg(*args, intmax_t);
        break;

    case LENGTH_Z: // 一般来说 size_t 和 ptrdiff_t 大小相等
    case LENGTH_T:
        value = va_arg(*args, ptrdiff_t);
        break;

    default:
        return 0;
    }

    // 判断正负
    if (value < 0)
    {
        *is_negative = true;
        return -value;
    }
    else
    {
        *is_negative = false;
        return value;
    }
}

/** 读取不同大小的无符号整数
 *
 * @param length 长度修饰符
 * @param args 可变参数列表
 * @return 返回读取的无符号整数值
 */
static uintmax_t get_va_arg_uint(enum LengthModifier length, va_list *args)
{
    switch (length)
    {
    case LENGTH_NONE:
    case LENGTH_HH:
    case LENGTH_H:
        return va_arg(*args, unsigned int);

    case LENGTH_L:
        return va_arg(*args, unsigned long);

    case LENGTH_LL:
        return va_arg(*args, unsigned long long);

    case LENGTH_J:
        return va_arg(*args, uintmax_t);

    case LENGTH_Z:
        return va_arg(*args, size_t);

    case LENGTH_T:
        return va_arg(*args, ptrdiff_t);

    default:
        return 0;
    }
}

/**
 * 将无符号整数转换为字符串
 *
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @param value 要转换的无符号整数值
 * @param precision 精度，表示最小位数
 * @param is_upper 是否使用大写字母表示十六进制数
 * @param base 进制，支持 8、10、16
 * @return 返回转换后的字符串长度
 */
static size_t uintmax_to_str(struct convert_args *args, uintmax_t value, bool is_upper, int base)
{
    static const char digits_lower[] = "0123456789abcdef";
    static const char digits_upper[] = "0123456789ABCDEF";

    if (base < 8 || base > 16)
        return 0;

    size_t i = 0;
    while (value > 0)
    {
        args->buffer[i] = (is_upper ? digits_upper : digits_lower)[value % base];
        value /= base;
        i++;
        if (i >= args->buf_size - 1) // buffer 太小
        {
            args->buffer[args->buf_size - 1] = 0;
            return args->buf_size - 1;
        }
    }

    // 如果数字长度小于精度，前面补 0
    for (int count = args->precision - i; count > 0; count--)
    {
        args->buffer[i] = '0';
        i++;
        if (i >= args->buf_size - 1) // buffer 太小
        {
            args->buffer[args->buf_size - 1] = 0;
            return args->buf_size - 1;
        }
    }

    return i;
}

/**
 * convert_int - 将有符号整数转换为字符串
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @param precision 精度，表示最小位数
 * @param flag 标志位
 * @param length 长度修饰符
 * @param is_upper 是否使用大写字母表示十六进制数
 * @param is_unsigned 是否为无符号整数
 * @param base 进制，支持 8~16
 * @param args 可变参数列表
 * @return 返回转换后的字符串长度
 */
static int convert_int(struct convert_args *fmt_args, bool is_upper, bool is_unsigned, int base, va_list *args)
{
    if (fmt_args->buf_size < 1)
        return -1;

    bool is_negative = false;
    intmax_t value = is_unsigned ? get_va_arg_uint(fmt_args->length, args) : get_va_arg_int(fmt_args->length, &is_negative, args);

    // 精度为 0，值为 0，输出空
    if (fmt_args->precision == 0 && value == 0)
    {
        fmt_args->buffer[0] = 0;
        return 0;
    }

    if (fmt_args->precision < 0)
        fmt_args->precision = 1; // 默认精度为 1

    size_t i = uintmax_to_str(fmt_args, value, is_upper, base);

    /* 添加符号 */
    if (!is_unsigned)
    {
        if ((is_negative || fmt_args->flag & FLAG_PLUS || fmt_args->flag & FLAG_SPACE) && i >= fmt_args->buf_size - 2) // 留一个给符号
        {
            fmt_args->buffer[fmt_args->buf_size - 1] = 0;
            return fmt_args->buf_size - 1;
        }
        if (is_negative)
            fmt_args->buffer[i++] = '-';
        else if (fmt_args->flag & FLAG_PLUS)
            fmt_args->buffer[i++] = '+';
        else if (fmt_args->flag & FLAG_SPACE)
            fmt_args->buffer[i++] = ' ';
    }

    int result_len = i;

    /* 反转字符串 */
    for (int j = 0; j < result_len / 2; j++)
    {
        char tmp;
        tmp = fmt_args->buffer[result_len - j - 1];
        fmt_args->buffer[result_len - j - 1] = fmt_args->buffer[j];
        fmt_args->buffer[j] = tmp;
    }

    fmt_args->buffer[result_len] = 0; // 末尾补 0

    return result_len;
}

/**
 * vsnprintf - 格式化输出到字符串
 * @buf: 输出缓冲区
 * @size: 缓冲区大小，包括末尾的 '\0'
 * @fmt: 格式字符串
 * @args: 可变参数列表
 */
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    if (size < 1)
        return -1;

    char *ptr = buf;
    char *end = buf + size - 1; // 留出一个位置给 '\0'

    if (end < buf) // 检查溢出
        end = (char *)SIZE_MAX;

    while (*fmt && ptr < end)
    {
        // 不为 % 直接复制
        if (*fmt != '%')
        {
            *ptr = *fmt;
            ptr++;
            fmt++;
            continue;
        }

        fmt++; // 跳过 %

        // % format 格式: %[标志][宽度][.精度][长度]类型
        struct convert_args fmt_args;

        // 读取 flag
        fmt_args.flag = 0;
        while (*fmt == '-' || *fmt == '+' || *fmt == ' ' || *fmt == '#' || *fmt == '0')
        {
            switch (*fmt)
            {
            case '-':
                fmt_args.flag |= FLAG_LEFT;
                break;
            case '+':
                fmt_args.flag |= FLAG_PLUS;
                break;
            case ' ':
                fmt_args.flag |= FLAG_SPACE;
                break;
            case '#':
                fmt_args.flag |= FLAG_HASH;
                break;
            case '0':
                fmt_args.flag |= FLAG_ZERO;
                break;
            default:
                write_wrong_format(&ptr, end);
                continue;
            }
            fmt++;
        }

        // 读取 width
        fmt_args.width = 0;
        if (*fmt == '*')
        {
            fmt_args.width = va_arg(args, int);
            fmt++;
        }
        else if (isdigit(*fmt) && parse_str_to_int(&fmt, &fmt_args.width))
        {
            write_wrong_format(&ptr, end);
            continue;
        }

        // 读取精度
        fmt_args.precision = 0;
        if (*fmt == '.')
        {
            fmt++;

            if (*fmt == '*')
            {
                fmt_args.precision = va_arg(args, int);
                fmt++;
            }
            else if (isdigit(*fmt) && parse_str_to_int(&fmt, &fmt_args.precision))
            {
                write_wrong_format(&ptr, end);
                continue;
            }

            // 如果 . 后面不跟数字，则精度为 0
        }
        else
        {
            fmt_args.precision = -1; // 使用默认精度
        }

        // 读取长度
        fmt_args.length = LENGTH_NONE;
        switch (*fmt)
        {
        case 'h':
            fmt++;
            if (*fmt == 'h')
            {
                fmt_args.length = LENGTH_HH;
                fmt++;
            }
            else
            {
                fmt_args.length = LENGTH_H;
            }
            break;
        case 'l':
            fmt++;
            if (*fmt == 'l')
            {
                fmt_args.length = LENGTH_LL;
                fmt++;
            }
            else
            {
                fmt_args.length = LENGTH_L;
            }
            break;
        case 'j':
            fmt_args.length = LENGTH_J;
            fmt++;
            break;
        case 'z':
            fmt_args.length = LENGTH_Z;
            fmt++;
            break;
        case 't':
            fmt_args.length = LENGTH_T;
            fmt++;
            break;
        case 'L':
            fmt_args.length = LENGTH_L_CAPITAL;
            fmt++;
            break;
        default:
            fmt_args.length = LENGTH_NONE;
            break;
        }

        // 处理类型
        int converted_len;
        char buffer[4096];
        fmt_args.buffer = buffer;
        fmt_args.buf_size = sizeof(buffer);

        switch (*fmt)
        {
        case 'd':
        case 'i':
            converted_len = convert_int(&fmt_args, false, false, 10, &args);
            break;

        case 'o':
            converted_len = convert_int(&fmt_args, false, true, 8, &args);
            break;

        case 'u':
            converted_len = convert_int(&fmt_args, false, true, 10, &args);
            break;

        case 'x':
            converted_len = convert_int(&fmt_args, false, true, 16, &args);
            break;

        case 'X':
            converted_len = convert_int(&fmt_args, true, true, 16, &args);
            break;

        case '%':
            fmt_args.buffer[0] = '%';
            fmt_args.buffer[1] = 0;
            converted_len = 1;
            break;

        default:
            write_wrong_format(&ptr, end);
            continue;
        }

        fmt++; // 跳过类型字符

            char pad_char = (fmt_args.flag & FLAG_ZERO) ? '0' : ' ';
        if (fmt_args.flag & FLAG_LEFT)
        {
            // 左对齐，在数字后面补填充
            for (; converted_len < fmt_args.width && converted_len < (int)sizeof(buffer); converted_len++)
            {
                fmt_args.buffer[converted_len] = pad_char;
            }
        }
        else
        {
            // 右对齐，在数字前面补填充
            for (int i = 0; i < fmt_args.width - converted_len && ptr < end; i++, ptr++)
            {
                *ptr = pad_char;
            }
        }

        // 复制缓冲区结果到 ptr
        for (int i = 0; i < converted_len && ptr < end; i++, ptr++)
        {
            *ptr = fmt_args.buffer[i];
        }
    }

    *ptr = 0; // 结尾补 0
    return ptr - buf;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
    return vsnprintf(buf, SIZE_MAX, fmt, args);
}

int sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vsprintf(buf, fmt, args);
    va_end(args);
    return ret;
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vsnprintf(buf, size, fmt, args);
    va_end(args);
    return ret;
}
