; kernel/boot/boot.asm


; Multiboot
MB_MAGIC        equ 0x1BADB002
MB_FLAGS        equ 0x03                    ; 4KB 对齐 + MMAP
MB_CHECKSUM     equ -(MB_MAGIC + MB_FLAGS)


[bits 32]

[section .multiboot align=4]
    dd MB_MAGIC
    dd MB_FLAGS
    dd MB_CHECKSUM


[section .boot.text]
global start
extern kmain, V_OFFSET, __bss_start, __bss_end

start:
    cli                                     ; 关闭中断

    lgdt [cs:gdtr_val]                      ; 加载 GDTR
    jmp 8:.clean_pipe                       ; 清除流水线
.clean_pipe:
    mov ax, 16
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov edi, mboot_info_addr                ; 存储 Multiboot info 结构体
    sub edi, V_OFFSET
    mov [edi], edx

    ; 清空 .bss
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    sub edi, V_OFFSET
    xor eax, eax
    rep stosb

    ; 配置 PTE0 和 PTEK, 内容相同
    xor eax, eax
    or eax, 0x003
    mov ebx, pte0
    sub ebx, V_OFFSET
    mov edi, ptek
    sub edi, V_OFFSET
    mov ecx, 1024
.write_pte:
    mov [ebx], eax
    mov [edi], eax
    add ebx, 4
    add edi, 4
    add eax, 0x1000
    loop .write_pte

    ; 配置 PDE 页目录表
    mov ebx, pde
    sub ebx, V_OFFSET

    mov eax, pte0
    sub eax, V_OFFSET
    or eax, 0x003                           ; RW + P
    mov [ebx + 0 * 4], eax

    mov eax, ptek
    sub eax, V_OFFSET
    or eax, 0x003
    mov ecx, V_OFFSET
    shr ecx, 22
    mov [ebx + ecx * 4], eax

    mov eax, ebx                            ; 使最后一个页目录项指向页目录表
    or eax, 0x003
    mov [ebx + 1023 * 4], eax               ; 0xFFC00000 ~ 0xFFFFFFFF

    mov eax, ebx
    mov cr3, eax                            ; 设置 CR3 寄存器

    add dword [gdtr_val + 2], V_OFFSET      ; 重新设置 gdt 的地址

    ; 打开 PG
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    lgdt [gdtr_val]
    jmp 8:.clean_pipe_again
.clean_pipe_again:
    mov ax, 16
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov gs, ax
    mov fs, ax

    mov eax, stack_top
    mov esp, eax                            ; 设置 ESP

    call kmain

.halt:
    cli
    hlt
    jmp .halt


[section .boot.data]
align 4
gdt_start:
    dq 0x0000000000000000                   ; #0 Empty segment
    dq 0x00CF9A000000FFFF                   ; #1 Code segment
    dq 0x00CF92000000FFFF                   ; #2 Data segment
gdt_end:
gdtr_val:
    dw gdt_end - gdt_start
    dd gdt_start


[section .bss]
alignb 4
stack_bottom:
    resb 16384                              ; 16 KB 栈空间
stack_top:

mboot_info_addr:                            ; Multiboot info 结构体地址
    resb 8

alignb 4096
pde:                                        ; 页目录表
    resb 4096

pte0:                                       ; 0 号页表 0x00000000 ~ 0x003FFFFF
    resb 4096
ptek:                                       ; 内核使用页表
    resb 4096
