// kernel/include/lib/math.h

#ifndef __LIB_MATH_H
#define __LIB_MATH_H

#include <stdint.h>
#include <stddef.h>

/* Need GNU C */
#ifndef __GNUC__
#error math.h needs GNU C
#endif

#define abs(a)              \
    ({                      \
        typeof(a) _a = (a); \
        _a > 0 ? _a : -_a   \
    })

#define min(a, b)           \
    ({                      \
        typeof(a) _a = (a); \
        typeof(b) _b = (b); \
        _a > _b ? _b : _a;  \
    })

#define max(a, b)           \
    ({                      \
        typeof(a) _a = (a); \
        typeof(b) _b = (b); \
        _a > _b ? _a : _b;  \
    })

#endif
