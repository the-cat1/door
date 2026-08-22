; SPDX-FileCopyrightText: 2026 Zhan Zixuan
;
; SPDX-License-Identifier: MIT

[bits 32]

[section .text]
extern task_schedule, selector_kernel_code
global task_switch_to, task_schedule_now

task_schedule_now:
    push ebp
    mov ebp, esp

    pushf
    push cs                                 ; cs
    push .ret                               ; ip
    push 0                                  ; errorcode
    push 0                                  ; irq
    push ds                                 ; segment registers
    push es
    push fs
    push gs
    pushad                                  ; 通用寄存器
    push esp                                ; 作为参数
    call task_schedule
    ud2
.ret:
    pop ebp
    ret

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
