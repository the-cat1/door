/* SPDX-FileCopyrightText: 2026 Zhan Zixuan
 *
 * SPDX-License-Identifier: MIT
 */

/* linker.ld */

ENTRY(_start)

SECTIONS
{
    . = 0x8000;

    .text : {
        *(.text.start) /* 把你的专属段放在最前面 */
        *(.text)
        *(.text.*)
    }

    .rodata : { *(.rodata*) }
    .data   : { *(.data*) }
    .bss    : {
        __bss_start = .;
        *(.bss) *(.COMMON)
        __bss_end = .;
    }
}
