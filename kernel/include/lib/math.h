// kernel/include/lib/math.h

#ifndef __LIB_MATH_H
#define __LIB_MATH_H

#include <stdint.h>
#include <stddef.h>

static inline char cabs(char i)
{
    return i > 0 ? i : -i;
}

static inline short sabs(short i)
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

static inline intmax_t imaxabs(intmax_t i)
{
    return i > 0 ? i : -i;
}

static inline ptrdiff_t pdabs(ptrdiff_t i)
{
    return i > 0 ? i : -i;
}

#endif
