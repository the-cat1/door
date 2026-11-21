// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include <stdbool.h>

#include "door/asm.h"

void main()
{
    *(unsigned char *)0xb8000 = '%';
    while (true)
        asm_hlt();
}
