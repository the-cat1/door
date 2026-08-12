// kernel/include/lib/math.h

#ifndef __LIB_MATH_H
#define __LIB_MATH_H

/* Need GNU C */
#ifndef __GNUC__
#error lib/math.h needs GNU C
#endif

#include "compiler.h"

/**
 * @brief 返回 a 的绝对值
 *
 * @warning 不要使用无符号数
 * @note 当输入为 INT_MIN (或类似) 时，将发生溢出
 */
#define abs(a)                         \
    ({                                 \
        typeof(a) _abs_a = (a);        \
        _abs_a < 0 ? -_abs_a : _abs_a; \
    })

/**
 * @brief 返回 a 和 b 中较小的那一个
 *
 * @warning 勿使用常数，要和常数比较，使用 minc
 * @note 这个宏会检查输入的类型是否相等
 */
#define min(a, b)                          \
    ({                                     \
        typeof(a) _min_a = (a);            \
        typeof(b) _min_b = (b);            \
        must_same_type(_min_a, _min_b);    \
        _min_a > _min_b ? _min_b : _min_a; \
    })

/**
 * @brief 返会 a, c 中较小的一个
 *
 * @warning c 必须为常数, 且类型会被强制转换成 a 的类型
 */
#define minc(a, c)                             \
    ({                                         \
        typeof(a) _minc_a = (a);               \
        typeof(a) _minc_c = (typeof(a))(c);    \
        _minc_a > _minc_c ? _minc_c : _minc_a; \
    })

/**
 * @brief 返回 a 和 b 中较大的那一个
 *
 * @warning 不要使用常数，要和常数比较，使用 maxc
 * @note 这个宏会检查输入的类型是否相等
 */
#define max(a, b)                          \
    ({                                     \
        typeof(a) _max_a = (a);            \
        typeof(b) _max_b = (b);            \
        must_same_type(_max_a, _max_b);    \
        _max_a > _max_b ? _max_a : _max_b; \
    })

/**
 * @brief 返会 a, c 中较大的一个
 *
 * @warning c 必须为常数, 且类型会被强制转换成 a 的类型
 */
#define maxc(a, c)                             \
    ({                                         \
        typeof(a) _maxc_a = (a);               \
        typeof(a) _maxc_c = (typeof(a))(c);    \
        _maxc_a > _maxc_c ? _maxc_a : _maxc_c; \
    })

#endif
