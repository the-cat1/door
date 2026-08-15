; SPDX-FileCopyrightText: 2026 Zhan Zixuan
;
; SPDX-License-Identifier: MIT

; idt的中断处理程序

[bits 32]

%define YEC nop             ; 有 errorcode, 无需做任何事
%define NEC push 0          ; 无 errorcode，压入 0 保持栈帧一致

%macro IRQ 2
[section .text]
IRQ%1:
    %2
    push %1
    jmp do_int

[section .rodata]
    dd IRQ%1
%endmacro

[section .text]
extern int_common

do_int:
    push gs                 ; 保存上下文
    push fs
    push es
    push ds
    push ss
    push ebp
    push esp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax
    mov ax, ss              ; 设置段寄存器
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    push esp                ; 压入栈帧地址
    call int_common         ; 调用函数
    add esp, 4              ; 弹出参数
    pop eax                 ; 恢复上下文
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop esp
    pop ebp
    pop ss
    pop ds
    pop es
    pop fs
    pop gs
    add esp, 8              ; 跳过中断号 + errorcode
    iret

[section .rodata]
global irq_proc_table
irq_proc_table:

; 内部中断
IRQ 0, NEC
IRQ 1, NEC
IRQ 2, NEC
IRQ 3, NEC
IRQ 4, NEC
IRQ 5, NEC
IRQ 6, NEC
IRQ 7, NEC
IRQ 8, YEC
IRQ 9, NEC
IRQ 10, YEC
IRQ 11, YEC
IRQ 12, YEC
IRQ 13, YEC
IRQ 14, YEC
IRQ 15, NEC
IRQ 16, NEC  ; Reserved
IRQ 17, YEC
IRQ 18, NEC
IRQ 19, NEC
IRQ 20, NEC  ; Reserved
IRQ 21, NEC  ; Reserved
IRQ 22, NEC  ; Reserved
IRQ 23, NEC  ; Reserved
IRQ 24, NEC  ; Reserved
IRQ 25, NEC  ; Reserved
IRQ 26, NEC  ; Reserved
IRQ 27, NEC  ; Reserved
IRQ 28, NEC  ; Reserved
IRQ 29, NEC  ; Reserved
IRQ 30, NEC  ; Reserved
IRQ 31, NEC  ; Reserved

; 外部中断
%assign vector 0x20
%rep 256 - 0x20
IRQ vector, NEC
%assign vector vector + 1
%endrep
