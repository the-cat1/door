// kernel/lib/ctype.h
// 提供字符相关的函数

#ifndef __LIB_CTYPE_H
#define __LIB_CTYPE_H

static inline int isalnum(int c);
static inline int isalpha(int c);
static inline int isascii(int c);
static inline int isblank(int c);
static inline int iscntrl(int c);
static inline int isdigit(int c);
static inline int isgraph(int c);
static inline int islower(int c);
static inline int isprint(int c);
static inline int ispunct(int c);
static inline int isspace(int c);
static inline int isupper(int c);
static inline int isxdigit(int c);

static inline int toascii(int c);
static inline int tolower(int c);
static inline int toupper(int c);

static inline int isalnum(int c)
{
    return isalpha(c) || isdigit(c);
}

static inline int isalpha(int c)
{
    return islower(c) || isupper(c);
}

static inline int isascii(int c)
{
    return c >= 0 && c <= 127;
}

static inline int isblank(int c)
{
    return c == ' ' || c == '\t';
}

static inline int iscntrl(int c)
{
    return (c >= 0 && c <= 31) || c == 127;
}

static inline int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

static inline int isgraph(int c)
{
    return isprint(c) && !isspace(c);
}

static inline int islower(int c)
{
    return c >= 'a' && c <= 'z';
}

static inline int isprint(int c)
{
    return c >= 32 && c <= 126;
}

static inline int ispunct(int c)
{
    return isgraph(c) && !isalnum(c);
}

static inline int isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static inline int isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}

static inline int isxdigit(int c)
{
    return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static inline int toascii(int c)
{
    return c & 0x7F;
}

static inline int tolower(int c)
{
    return isupper(c) ? (c + 0x20) : c;
}

static inline int toupper(int c)
{
    return islower(c) ? (c - 0x20) : c;
}

#endif
