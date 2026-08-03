// kernel/include/lib/math.h

#ifndef __LIB_MATH_H
#define __LIB_MATH_H

#include <stdint.h>
#include <stddef.h>

static inline char hhabs(char i)
{
    return i > 0 ? i : -i;
}

static inline short habs(short i)
{
    return i > 0 ? i : -i;
}

static inline int abs(int i)
{
    return i > 0 ? i : -i;
}

static inline long labs(long i)
{
    return i > 0 ? i : -i;
}

static inline long long llabs(long long i)
{
    return i > 0 ? i : -i;
}

static inline intmax_t jabs(intmax_t i)
{
    return i > 0 ? i : -i;
}

static inline ptrdiff_t pabs(ptrdiff_t i)
{
    return i > 0 ? i : -i;
}

#endif
