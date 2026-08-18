; SPDX-FileCopyrightText: 2026 Zhan Zixuan
;
; SPDX-License-Identifier: MIT

[bits 32]

[section .text]
global task_switch_to

task_switch_to:
    mov eax, [esp + 4]
    mov esp, [eax + 8]
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 8
    iret
