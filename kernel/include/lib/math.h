// kernel/include/lib/math.h

#ifndef __LIB_MATH_H
#define __LIB_MATH_H

#include <stdint.h>
#include <stddef.h>

#define abs(a) ((a) > 0 ? (a) : -(a))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#endif
