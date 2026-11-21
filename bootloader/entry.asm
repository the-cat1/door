; SPDX-FileCopyrightText: 2026 Zhan Zixuan
;
; SPDX-License-Identifier: MIT


global _start
extern main, __bss_start, __bss_end

section .text.start
[bits 16]
_start:
    cli                 ; no interrupt
    cld                 ; clear direction flag

    ; Set segment and stack registers.
    xor ax, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov sp, 0x8000

    ; Open A20 gate
    in al, 0x92
    or al, 0x02
    and al, 0xFE        ; avoid soft reset
    out 0x92, al

    ; Load GDTR
    lgdt [gdt_descriptor]

    ; Enter Protect Mode by setting CR0.PE = 1
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    ; Long jump to clear pipe
    jmp CODE_SEG:clear_pipe

[bits 32]
clear_pipe:
    ; Set 32-bits segment registers
    mov ax, DATA_SEG
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ds, ax

    ; Set up 32-bits stack.
    mov esp, 0x8000
    mov ebp, esp

    ; Zero BSS.
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ;  Jump to C codes.
    call main

.hang:
    cli
    hlt
    jmp .hang

section .rodata
; Global Descriptor Table
align 8
gdt_start:
gdt_0     dq 0x0000000000000000
gdt_code  dq 0x00cf9a000000ffff
gdt_data  dq 0x00cf92000000ffff
gdt_end:

; the value of GDTR
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; const selector
CODE_SEG  equ gdt_code - gdt_start
DATA_SEG  equ gdt_data - gdt_start

