// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

// 提供一些编译器相关的宏

#ifndef __COMPILER_H
#define __COMPILER_H

/* Need GNU C */
#ifndef __GNUC__
#error compiler.h needs GNU C
#endif

#ifndef static_assert
#define _static_assert(expr, msg)  _Static_assert((expr), msg)
#define static_assert(expr) _static_assert(expr, #expr)
#endif

#define same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define must_same_type(a, b) static_assert(same_type((a), (b)))
#define must_diff_type(a, b) static_assert(!same_type((a), (b)))

#endif
