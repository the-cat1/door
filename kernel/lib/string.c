// kernel/lib/string.c

#include <stddef.h>

#include "lib/string.h"

/* --- memory functions --- */

/**
 * @brief 从源内存复制 n 个字节到目标内存
 *
 * @param s1 目标内存指针
 * @param s2 源内存指针
 * @param n 要复制的字节数
 * @return void* 目标内存指针 s1
 * @note 源与目标内存不能重叠, 需要处理重叠时请使用 memmove
 * @note 当 s1 或 s2 为 NULL 时返回 NULL
 */
void *memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
    if (!s1 || !s2)
        return NULL;

    for (size_t i = 0; i < n; i++)
        ((char *)s1)[i] = ((char *)s2)[i];

    return s1;
}

/**
 * @brief 从源内存复制 n 个字节到目标内存, 可处理内存区域重叠
 *
 * @param s1 目标内存指针
 * @param s2 源内存指针
 * @param n 要复制的字节数
 * @return void* 目标内存指针 s1
 * @note 当 s1 或 s2 为 NULL 时返回 NULL
 */
void *memmove(void *s1, const void *s2, size_t n)
{
    if (!s1 || !s2)
        return NULL;

    char *d = (char *)s1;
    const char *s = (const char *)s2;

    /* Handle overlapping regions correctly. */
    if (d < s)
    {
        for (size_t i = 0; i < n; i++)
            d[i] = s[i];
    }
    else if (d > s)
    {
        for (size_t i = n; i > 0; i--)
            d[i - 1] = s[i - 1];
    }

    return s1;
}

/**
 * @brief 用指定值填充内存区域的前 n 个字节
 *
 * @param s 目标内存指针
 * @param c 填充值, 会被转换为 unsigned char
 * @param n 要填充的字节数
 * @return void* 目标内存指针 s
 * @note 当 s 为 NULL 时返回 NULL
 */
void *memset(void *s, int c, size_t n)
{
    if (!s)
        return NULL;

    unsigned char *p = (unsigned char *)s;
    unsigned char uc = (unsigned char)c;

    for (size_t i = 0; i < n; i++)
        p[i] = uc;

    return s;
}

/**
 * @brief 逐字节比较两块内存区域的前 n 个字节
 *
 * @param s1 第一块内存指针
 * @param s2 第二块内存指针
 * @param n 要比较的字节数
 * @return int 若 s1 < s2 返回负数, 相等返回 0, 若 s1 > s2 返回正数
 * @note 当 s1 或 s2 为 NULL 时返回 0
 */
int memcmp(const void *s1, const void *s2, size_t n)
{
    if (!s1 || !s2)
        return 0;

    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (a[i] != b[i])
            return (int)a[i] - (int)b[i];
    }

    return 0;
}

/**
 * @brief 在内存区域的前 n 个字节中查找指定字符第一次出现的位置
 *
 * @param s 要查找的内存区域指针
 * @param c 要查找的字符, 会被转换为 unsigned char
 * @param n 要查找的字节数
 * @return void* 指向匹配字符的指针; 未找到时返回 NULL
 * @note 当 s 为 NULL 时返回 NULL
 */
void *memchr(const void *s, int c, size_t n)
{
    if (!s)
        return NULL;

    const unsigned char *p = (const unsigned char *)s;
    unsigned char uc = (unsigned char)c;

    for (size_t i = 0; i < n; i++)
    {
        if (p[i] == uc)
            return (void *)(p + i);
    }

    return NULL;
}

/* --- copy / concatenate functions --- */

/**
 * @brief 将字符串 s2 (含结尾 `\0`) 复制到 s1
 *
 * @param s1 目标缓冲区指针, 必须足够大以容纳 s2
 * @param s2 源字符串指针
 * @return char* 目标字符串指针 s1
 * @note 源与目标区域不能重叠, 否则行为未定义
 * @note 当 s1 或 s2 为 NULL 时返回 NULL
 */
char *strcpy(char *restrict s1, const char *restrict s2)
{
    if (!s1 || !s2)
        return NULL;

    char *d = s1;

    while ((*d++ = *s2++))
        ;

    return s1;
}

/**
 * @brief 将源字符串 s2 最多 n 个字符复制到 s1
 *
 * @param s1 目标缓冲区指针
 * @param s2 源字符串指针
 * @param n 最大复制字符数
 * @return char* 目标字符串指针 s1
 * @note 若 s2 长度小于 n, 剩余部分用 `\0` 填充
 * @note 当 s1 或 s2 为 NULL 时返回 NULL
 */
char *strncpy(char *restrict s1, const char *restrict s2, size_t n)
{
    if (!s1 || !s2)
        return NULL;

    char *d = s1;

    while (n && (*d = *s2))
    {
        d++;
        s2++;
        n--;
    }

    /* Pad with null bytes if s2 is shorter than n. */
    while (n--)
    {
        *d = '\0';
        d++;
    }

    return s1;
}

/**
 * @brief 将字符串 s2 追加到 s1 的末尾
 *
 * @param s1 目标字符串指针, 必须以 `\0` 结尾且缓冲区足够大
 * @param s2 要追加的源字符串指针
 * @return char* 目标字符串指针 s1
 * @note 当 s1 或 s2 为 NULL 时返回 NULL
 */
char *strcat(char *restrict s1, const char *restrict s2)
{
    if (!s1 || !s2)
        return NULL;

    char *d = s1;

    while (*d)
        d++;
    while ((*d++ = *s2++))
        ;

    return s1;
}

/**
 * @brief 将字符串 s2 最多 n 个字符追加到 s1 的末尾
 *
 * @param s1 目标字符串指针, 必须以 `\0` 结尾且缓冲区足够大
 * @param s2 要追加的源字符串指针
 * @param n 最大追加字符数
 * @return char* 目标字符串指针 s1
 * @note 无论是否达到 n, 追加完成后都会补上结尾 `\0`
 * @note 当 s1 或 s2 为 NULL 时返回 NULL
 */
char *strncat(char *restrict s1, const char *restrict s2, size_t n)
{
    if (!s1 || !s2)
        return NULL;

    char *d = s1;

    while (*d)
        d++;
    while (n-- && (*d = *s2))
    {
        d++;
        s2++;
    }
    *d = '\0';

    return s1;
}

/* --- compare functions --- */

/**
 * @brief 比较两个字符串
 *
 * @param s1 第一个字符串指针
 * @param s2 第二个字符串指针
 * @return int 若 s1 < s2 返回负数, 相等返回 0, 若 s1 > s2 返回正数
 * @note 比较基于 unsigned char 的值
 * @note 当 s1 或 s2 为 NULL 时返回 0
 */
int strcmp(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return 0;

    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }

    return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

/**
 * @brief 按当前 locale 比较两个字符串
 *
 * @param s1 第一个字符串指针
 * @param s2 第二个字符串指针
 * @return int 若 s1 < s2 返回负数, 相等返回 0, 若 s1 > s2 返回正数
 * @note 内核未提供 locale 支持, 因此行为与 strcmp 相同
 * @note 当 s1 或 s2 为 NULL 时返回 0
 */
int strcoll(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return 0;

    /* No locale support in the kernel; identical to strcmp. */
    return strcmp(s1, s2);
}

/**
 * @brief 比较两个字符串的前 n 个字符
 *
 * @param s1 第一个字符串指针
 * @param s2 第二个字符串指针
 * @param n 最大比较字符数
 * @return int 若 s1 < s2 返回负数, 相等返回 0, 若 s1 > s2 返回正数
 * @note 当 s1 或 s2 为 NULL 时返回 0
 */
int strncmp(const char *s1, const char *s2, size_t n)
{
    if (!s1 || !s2)
        return 0;

    for (; n > 0; n--, s1++, s2++)
    {
        if (*s1 != *s2)
            return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
        if (*s1 == '\0')
            return 0;
    }

    return 0;
}

/**
 * @brief 将字符串 s2 按当前 locale 规则变换后写入 s1
 *
 * @param s1 目标缓冲区指针
 * @param s2 源字符串指针
 * @param n 目标缓冲区的可用大小
 * @return size_t 变换后字符串的长度, 不包括结尾 `\0`
 * @note 内核未提供 locale 支持, 因此变换为恒等; 若 n 为 0, s1 可传 NULL
 * @note 当 s2 为 NULL 时返回 0
 */
size_t strxfrm(char *restrict s1, const char *restrict s2, size_t n)
{
    /* No locale support; the transform is the identity. */
    size_t len = 0;
    const char *p = s2;

    if (!p || (n > 0 && !s1))
        return 0;

    while (*p++)
        len++;

    if (n > 0)
    {
        size_t i = 0;
        while (i < len && i < n - 1)
        {
            s1[i] = s2[i];
            i++;
        }
        s1[i] = '\0';
    }

    return len;
}

/* --- search functions --- */

/**
 * @brief 在字符串中查找指定字符第一次出现的位置
 *
 * @param s 要查找的字符串指针
 * @param c 要查找的字符
 * @return char* 指向匹配字符的指针; 未找到时返回 NULL
 * @note 也会匹配字符串结尾的 `\0`
 * @note 当 s 为 NULL 时返回 NULL
 */
char *strchr(const char *s, int c)
{
    if (!s)
        return NULL;

    char ch = (char)c;

    for (;; s++)
    {
        if (*s == ch)
            return (char *)s;
        if (*s == '\0')
            return NULL;
    }
}

/**
 * @brief 在字符串中从末尾向前查找指定字符
 *
 * @param s 要查找的字符串指针
 * @param c 要查找的字符
 * @return char* 指向最后一次匹配的字符的指针; 未找到时返回 NULL
 * @note 也会匹配字符串结尾的 `\0`
 * @note 当 s 为 NULL 时返回 NULL
 */
char *strrchr(const char *s, int c)
{
    if (!s)
        return NULL;

    char ch = (char)c;
    const char *found = NULL;

    for (;; s++)
    {
        if (*s == ch)
            found = s;
        if (*s == '\0')
            break;
    }

    return (char *)found;
}

/**
 * @brief 计算 s1 开头仅由 s2 中的字符组成的最大前缀长度
 *
 * @param s1 被检查的字符串指针
 * @param s2 字符集合字符串指针
 * @return size_t 满足条件的前缀长度
 * @note 当 s1 或 s2 为 NULL 时返回 0
 */
size_t strspn(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return 0;

    size_t n = 0;
    const char *p;

    for (p = s1; *p; p++)
    {
        const char *q;
        for (q = s2; *q; q++)
        {
            if (*p == *q)
                break;
        }
        if (!*q)
            break;
        n++;
    }

    return n;
}

/**
 * @brief 计算 s1 开头不包含 s2 中任何字符的最大前缀长度
 *
 * @param s1 被检查的字符串指针
 * @param s2 字符集合字符串指针
 * @return size_t 满足条件的前缀长度
 * @note 当 s1 或 s2 为 NULL 时返回 0
 */
size_t strcspn(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return 0;

    size_t n = 0;
    const char *p;

    for (p = s1; *p; p++)
    {
        const char *q;
        for (q = s2; *q; q++)
        {
            if (*p == *q)
                return n;
        }
        n++;
    }

    return n;
}

/**
 * @brief 在 s1 中查找 s2 中任意字符第一次出现的位置
 *
 * @param s1 被查找的字符串指针
 * @param s2 字符集合字符串指针
 * @return char* 指向匹配字符的指针; 未找到时返回 NULL
 * @note 当 s1 或 s2 为 NULL 时返回 NULL
 */
char *strpbrk(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return NULL;

    const char *p;

    for (p = s1; *p; p++)
    {
        const char *q;
        for (q = s2; *q; q++)
        {
            if (*p == *q)
                return (char *)p;
        }
    }

    return NULL;
}

/**
 * @brief 在 s1 中查找子串 s2 第一次出现的位置
 *
 * @param s1 被查找的字符串指针
 * @param s2 要查找的子串指针
 * @return char* 指向子串首字符的指针; 未找到时返回 NULL
 * @note 若 s2 为空字符串, 直接返回 s1
 * @note 当 s1 或 s2 为 NULL 时返回 NULL
 */
char *strstr(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return NULL;

    const char *p;

    if (!*s2)
        return (char *)s1;

    for (p = s1; *p; p++)
    {
        const char *a = p;
        const char *b = s2;

        while (*a && *b && (*a == *b))
        {
            a++;
            b++;
        }
        if (!*b)
            return (char *)p;
    }

    return NULL;
}

/**
 * @brief 按分隔符集合将字符串分割为一系列片段
 *
 * @param s1 要分割的字符串指针; 后续调用传 NULL 表示继续上一次分割
 * @param s2 分隔符集合字符串指针
 * @return char* 指向下一个片段的指针; 没有更多片段时返回 NULL
 * @note 内部使用静态变量保存状态, 不可重入; 同一字符串的分割过程会修改 s1 内容
 * @note 当 s2 为 NULL 时返回 NULL
 */
char *strtok(char *restrict s1, const char *restrict s2)
{
    static char *save = NULL;
    char *token;

    if (!s2)
        return NULL;

    if (s1)
        save = s1;

    if (!save)
        return NULL;

    /* Skip leading delimiters. */
    while (*save && strchr(s2, *save))
        save++;

    if (!*save)
    {
        save = NULL;
        return NULL;
    }

    token = save;

    /* Find end of token. */
    while (*save && !strchr(s2, *save))
        save++;

    if (*save)
    {
        *save = '\0';
        save++;
    }
    else
    {
        save = NULL;
    }

    return token;
}

/* --- length functions --- */

/**
 * @brief 获取字符串的长度
 *
 * @param s 字符串
 * @return size_t 字符串的长度，不包括 `\0`
 * @note 当输入为 NULL 时返回 0, 避免崩溃
 */
size_t strlen(const char *s)
{
    if (!s) // 扩展行为
        return 0;

    size_t len = 0;
    while (*s++)
        len++;

    return len;
}
