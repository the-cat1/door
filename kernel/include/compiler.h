// kernel/include/compiler.h

#ifndef __COMPILER_H
#define __COMPILER_H

/* Need GNU C */
#ifndef __GNUC__
#error compiler.h needs GNU C
#endif

#define static_assert _Static_assert /* Avoid some compiler does not support static_assert without <assert.h> */

#define same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define must_same_type(a, b) static_assert(same_type((a), (b)))
#define must_diff_type(a, b) static_assert(!same_type((a), (b)))

#endif
