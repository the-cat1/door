// kernel/include/assert.h
// 提供 assert 宏

#ifndef __ASSERT_H
#define __ASSERT_H

#include "panic.h"

#ifdef NDEBUG
#define assert(ignore) ((void)0)
#else
#define assert(expr) ((expr) ? ((void)0) : panic("assert `%s` fail", #expr))
#endif /* NDEBUG */

#endif
