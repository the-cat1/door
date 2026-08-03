// kernel/include/lib/math.h

#ifndef __LIB_MATH_H
#define __LIB_MATH_H

#include <stdint.h>
#include <stddef.h>

#define ABS(a) ((a) > 0 ? (a) : -(a))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#endif
