// kernel/lib/sprintf.c

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "lib/ctype.h"
#include "lib/string.h"
#include "lib/math.h"
#include "lib/sprintf.h"

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
    char **ptr;
    char *end;
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
    bool is_negative = false;

    if (**str == '-')
    {
        (*str)++;
        is_negative = true;
    }

    while (isdigit(**str))
    {
        buf[i] = **str;
        if (i++ >= 12)
            return -1; // 数字太长
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

    if (is_negative)
        *value = -*value;

    return 0;
}

/**
 * 读取不同大小的有符号整数
 *
 * @param fmt_args fmt 参数列表
 * @return 返回读取的整数值
 */
static intmax_t get_va_arg_int(struct convert_args *fmt_args)
{
    switch (fmt_args->length)
    {
    case LENGTH_HH: // 对于 char 和 short，实际 va_list 里是 int 类型
    case LENGTH_H:
    case LENGTH_NONE:
        return va_arg(*fmt_args->args, int);

    case LENGTH_L:
        return va_arg(*fmt_args->args, long);

    case LENGTH_LL:
        return va_arg(*fmt_args->args, long long);

    case LENGTH_J:
        return va_arg(*fmt_args->args, intmax_t);

    case LENGTH_Z: // 一般来说 size_t 和 ptrdiff_t 大小相等
    case LENGTH_T:
        return va_arg(*fmt_args->args, ptrdiff_t);

    default:
        return 0;
    }
}

/**
 * 读取不同大小的无符号整数
 *
 * @param length 长度修饰符
 * @param args 可变参数列表
 * @return 返回读取的无符号整数值
 */
static uintmax_t get_va_arg_uint(struct convert_args *fmt_args)
{
    switch (fmt_args->length)
    {
    case LENGTH_NONE:
    case LENGTH_HH:
    case LENGTH_H:
        return va_arg(*fmt_args->args, unsigned int);

    case LENGTH_L:
        return va_arg(*fmt_args->args, unsigned long);

    case LENGTH_LL:
        return va_arg(*fmt_args->args, unsigned long long);

    case LENGTH_J:
        return va_arg(*fmt_args->args, uintmax_t);

    case LENGTH_Z:
        return va_arg(*fmt_args->args, size_t);

    case LENGTH_T:
        return va_arg(*fmt_args->args, ptrdiff_t);

    default:
        return 0;
    }
}

/**
 * 将无符号整数转换为字符串（反转形式）
 *
 * @param fmt_args 格式化参数
 * @param value 要转换的无符号整数值
 * @param is_upper 是否使用大写字母表示十六进制数
 * @param base 进制，支持 8、10、16
 * @return 返回转换后的字符串长度
 */
static size_t uintmax_to_str(struct convert_args *fmt_args, uintmax_t value, bool is_upper, int base)
{
    static const char digits_lower[] = "0123456789abcdef";
    static const char digits_upper[] = "0123456789ABCDEF";

    if (base < 8 || base > 16)
        return 0;

    // 将 value 转换成反转的字符串
    char *p = *fmt_args->ptr;
    int i = 0;
    while (value > 0 && p + i < fmt_args->end)
    {
        p[i++] = (is_upper ? digits_upper : digits_lower)[value % base];
        value /= base;
    }

    // 如果是 8 进制且有 HASH 标志，需要增加精度来添加 0 前缀
    if (base == 8 && fmt_args->flag & FLAG_HASH)
        fmt_args->precision = MAX(fmt_args->precision, i + 1);

    // 如果数字长度小于精度，前面补 0 或 ‘ ’ (空格)
    int pad_size = fmt_args->precision - i;
    for (int j = 0; j < pad_size && p + i < fmt_args->end; j++)
        p[i++] = '0';

    // 如果是非零 16 进制并且有 HASH，需要添加 0x / 0X 前缀
    if (i != 0 && base == 16 && fmt_args->flag & FLAG_HASH)
    {
        p[i++] = is_upper ? 'X' : 'x';
        p[i++] = '0';
    }

    return i;
}

/**
 * @brief 将有符号整数转换为字符串
 *
 * @param fmt_args 格式化参数
 * @param is_upper 是否使用大写字母表示十六进制数
 * @param is_unsigned 是否为无符号整数
 * @param base 进制，支持 8~16
 * @return 返回转换后的字符串长度
 */
static int convert_int(struct convert_args *fmt_args, bool is_upper, bool is_unsigned, int base)
{
    uintmax_t uvalue;
    bool is_negative = false;

    /* 根据有没有符号获取参数 */
    if (is_unsigned)
    {
        uvalue = get_va_arg_uint(fmt_args);
    }
    else
    {
        intmax_t value = get_va_arg_int(fmt_args);
        is_negative = value < 0;
        if (is_negative)
            uvalue = (uintmax_t)-value;
        else
            uvalue = (uintmax_t)value;
    }

    char *ptr = *fmt_args->ptr;

    bool have_sign = fmt_args->flag & (FLAG_SPACE | FLAG_PLUS) || is_negative;

    if (fmt_args->precision < 0)
    {
        if (fmt_args->flag & FLAG_ZERO) // 指定了 `0` 标志，且没有指定精度，需要填充 0 到宽度
            fmt_args->precision = have_sign ? fmt_args->width - 1 : fmt_args->width;
        else
            fmt_args->precision = 1; // 默认精度为 1
    }

    size_t i = uintmax_to_str(fmt_args, (uintmax_t)uvalue, is_upper, base);

    /* 添加符号 */
    if (!is_unsigned)
    {
        if (have_sign && i >= (size_t)(fmt_args->end - ptr - 1)) // 留一个给符号
        {
            ptr[i] = 0;
            return i;
        }

        if (is_negative)
            ptr[i++] = '-';
        else if (fmt_args->flag & FLAG_PLUS)
            ptr[i++] = '+';
        else if (fmt_args->flag & FLAG_SPACE)
            ptr[i++] = ' ';
    }

    int result_len = i;

    /* 反转字符串 */
    for (int j = 0; j < result_len / 2; j++)
    {
        char tmp;
        tmp = ptr[result_len - j - 1];
        ptr[result_len - j - 1] = ptr[j];
        ptr[j] = tmp;
    }

    return result_len;
}

static int convert_string(struct convert_args *fmt_args)
{
    char *str = va_arg(*fmt_args->args, char *);

    if (!str)
        str = "[NULL]";

    if (fmt_args->precision < 0)
        fmt_args->precision = INT32_MAX;

    size_t copy_len = MIN((size_t)MIN(fmt_args->end - *fmt_args->ptr, fmt_args->precision), strlen(str));
    memcpy(*fmt_args->ptr, str, copy_len);

    return copy_len;
}

static int convert_pointer(struct convert_args *fmt_args)
{
    fmt_args->flag = FLAG_ZERO | FLAG_HASH;
    fmt_args->width = 32 / 4;
    fmt_args->length = LENGTH_Z;

    return convert_int(fmt_args, false, true, 16);
}

// 往 ptr 写 pad_size 个填充符号
static void pad(struct convert_args *fmt_args, int pad_size)
{
    for (int i = 0; i < pad_size && *fmt_args->ptr < fmt_args->end; i++)
        *(*fmt_args->ptr)++ = ' ';
}

// 解析一个 % 格式化字符串，返回 0 正常，其他则为错误
static int parse_fmt(struct convert_args *fmt_args, char **ptr, char *end, const char **fmt)
{
    // 读取 flag
    fmt_args->flag = 0;
    while (**fmt == '-' || **fmt == '+' || **fmt == ' ' || **fmt == '#' || **fmt == '0')
    {
        switch (**fmt)
        {
        case '-':
            fmt_args->flag |= FLAG_LEFT;
            break;
        case '+':
            fmt_args->flag |= FLAG_PLUS;
            break;
        case ' ':
            fmt_args->flag |= FLAG_SPACE;
            break;
        case '#':
            fmt_args->flag |= FLAG_HASH;
            break;
        case '0':
            fmt_args->flag |= FLAG_ZERO;
            break;
        default:
            // Impossible!!!
            break;
        }
        (*fmt)++;
    }

    // 读取 width
    fmt_args->width = 0;
    if (**fmt == '*')
    {
        (*fmt)++;
        fmt_args->width = va_arg(*fmt_args->args, int);
    }
    else if (isdigit(**fmt) && parse_str_to_int(fmt, &fmt_args->width))
    {
        write_wrong_format(ptr, end);
        return -1;
    }

    if (fmt_args->width < 0)
    {
        fmt_args->width = -fmt_args->width;
        fmt_args->flag |= FLAG_LEFT;
    }

    // 读取精度
    fmt_args->precision = -1;
    if (**fmt == '.')
    {
        (*fmt)++;

        if (**fmt == '*')
        {
            fmt_args->precision = va_arg(*fmt_args->args, int);
            (*fmt)++;
        }
        else if (isdigit(**fmt) || **fmt == '-')
        {
            if (parse_str_to_int(fmt, &fmt_args->precision))
            {
                write_wrong_format(ptr, end); // parse 失败
                return -1;
            }
        }
        else
        {
            // 如果 . 后面不跟数字，则精度为 0
            fmt_args->precision = 0;
        }
    }

    // 读取长度
    switch (**fmt)
    {
    case 'h':
        (*fmt)++;
        if (**fmt == 'h')
        {
            fmt_args->length = LENGTH_HH;
            (*fmt)++;
        }
        else
        {
            fmt_args->length = LENGTH_H;
        }
        break;

    case 'l':
        (*fmt)++;
        if (**fmt == 'l')
        {
            fmt_args->length = LENGTH_LL;
            (*fmt)++;
        }
        else
        {
            fmt_args->length = LENGTH_L;
        }
        break;

    case 'j':
        fmt_args->length = LENGTH_J;
        (*fmt)++;
        break;

    case 'z':
        fmt_args->length = LENGTH_Z;
        (*fmt)++;
        break;

    case 't':
        fmt_args->length = LENGTH_T;
        (*fmt)++;
        break;

    case 'L':
        fmt_args->length = LENGTH_L_CAPITAL;
        (*fmt)++;
        break;

    default:
        fmt_args->length = LENGTH_NONE;
        break;
    }

    return 0;
}

static void write_n(struct convert_args *fmt_args, ptrdiff_t value)
{
    void *ptr = va_arg(*fmt_args->args, void *);

    if (!ptr)
        return;

    switch (fmt_args->length)
    {
    case LENGTH_NONE:
        *(int *)ptr = value;
        break;

    case LENGTH_HH:
        *(signed char *)ptr = value;
        break;

    case LENGTH_H:
        *(short *)ptr = value;
        break;

    case LENGTH_L:
        *(long *)ptr = value;
        break;

    case LENGTH_LL:
        *(long long *)ptr = value;
        break;

    case LENGTH_J:
        *(intmax_t *)ptr = value;
        break;

    case LENGTH_Z:
        *(size_t *)ptr = (size_t)value;
        break;

    case LENGTH_T:
        *(ptrdiff_t *)ptr = value;
        break;

    default:
        break;
    }
}

/**
 * @brief 格式化输出到字符串
 *
 * @param buf 输出缓冲区
 * @param size 缓冲区大小，包括末尾的 '\0'
 * @param fmt 格式字符串
 * @param args 可变参数列表
 * @return 写入的字节数
 *
 * 支持 %i %d %u %o %x %X %c %s %p
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

        int converted_len;
        struct convert_args fmt_args = {
            .args = &args,
            .ptr = &ptr,
            .end = end};

        if (parse_fmt(&fmt_args, &ptr, end, &fmt))
            continue;

        // 处理类型
        switch (*fmt)
        {
        case 'd': // 十进制有符号整数
        case 'i':
            converted_len = convert_int(&fmt_args, false, false, 10);
            break;

        case 'o': // 八进制无符号整数
            converted_len = convert_int(&fmt_args, false, true, 8);
            break;

        case 'u': // 十进制无符号整数
            converted_len = convert_int(&fmt_args, false, true, 10);
            break;

        case 'x': // 十六进制无符号整数 小写
            converted_len = convert_int(&fmt_args, false, true, 16);
            break;

        case 'X': // 十六进制无符号整数 大写
            converted_len = convert_int(&fmt_args, true, true, 16);
            break;

        case 's':
            converted_len = convert_string(&fmt_args);
            break;

        case 'c':
            ptr[0] = get_va_arg_int(&fmt_args); // 简单地使用 int 的长度描述符，实际上只有 `l` -> wint_t
            converted_len = 1;
            break;

        case 'p':
            converted_len = convert_pointer(&fmt_args);
            break;

        case 'n': // 将已写入的字符数存储到参数中
            write_n(&fmt_args, ptr - buf);
            fmt++; // 用于跳过错误的格式字符 否则会多一个字
            continue;

        case '%':
            *(ptr++) = '%';
            fmt++;    // 同上, 跳过格式字符
            continue; // 此时会检查 ptr < end

        default:
            write_wrong_format(&ptr, end);
            fmt++;
            continue;
        }

        fmt++; // 跳过类型字符

        converted_len = MIN(converted_len, end - ptr);
        fmt_args.width = MIN(fmt_args.width, end - ptr);
        int pad_size = MAX(fmt_args.width - converted_len, 0);

        if (fmt_args.flag & FLAG_LEFT)
        {
            ptr += converted_len;
            pad(&fmt_args, pad_size);
        }
        else
        {
            memmove(ptr + pad_size, ptr, converted_len);
            pad(&fmt_args, pad_size);
            ptr += converted_len;
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
