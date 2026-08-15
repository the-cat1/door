// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#ifndef __ARRAY_SIZE_H
#define __ARRAY_SIZE_H

#include "compiler.h"

#define must_be_array(arr) must_diff_type((arr), &(arr)[0])
#define array_size(arr)               \
    ({                                \
        must_be_array(arr);           \
        sizeof(arr) / sizeof(arr[0]); \
    })

#endif
