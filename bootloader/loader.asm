; SPDX-FileCopyrightText: 2026 Zhan Zixuan
;
; SPDX-License-Identifier: MIT

; 内核加载器，不完全的 Multiboot 支持
; 仅适用于 1.44MB 软盘，FAT12 文件系统


SP_ADDR             equ 0x07e00

MB_INFO_ADDR        equ 0x02000     ; Multiboot 启动信息地址
MB_MEM_INFO_ADDR    equ 0x04000     ; Multiboot 内存信息地址
CFG_PARSE_KEY_ADDR  equ 0x81000     ; CFG 解析 KEY 缓冲区
CFG_PARSE_BUF_SIZE  equ 0xfac       ; CFG 解析 缓冲区大小
CFG_BOOT_NAME       equ 0x82000     ; boot_name
CFG_KERNEL_PATH     equ 0x83000     ; kernel_path
CFG_KERNEL_CMDLINE  equ 0x84000     ; kernel_cmdline

FAT_ADDR            equ 0x30000     ; 读取 FAT 表到这个地址
KERNEL_HEAD_ADDR    equ 0x40000     ; 内核文件前 32768 字节读取的位置
KERNEL_HEAD_SIZE    equ 32768       ; 在文件的前 32768 字节查找 Multiboot head
KERNEL_BUF_ADDR     equ 0x4C000     ; KERNEL 缓冲区
ROOT_ADDR           equ 0x50000     ; 根目录加载位置
DIR_BUFFER_ADDR     equ 0x60000     ; 子目录加载位置
CFG_FILE_ADDR       equ 0x70000     ; 配置文件加载地址
CFG_FILE_SIZE       equ 0x0F000     ; 配置文件最大大小

GDT_ADDR            equ 0x10000     ; GDT 地址 (需为16的整数倍)
GDT_DATA_SEG        equ 1 << 3      ; 1# 数据段 0~4GB RW
GDT_DATA_ENTRY      equ 0x00CF92000000FFFF
GDT_CODE_SEG        equ 2 << 3      ; 2# 代码段 0~4GB XR
GDT_CODE_ENTRY      equ 0x00CF9A000000FFFF
GDT_SIZE            equ 3 * 8 - 1

IDT_ADDR            equ 0x20000     ; IDT 地址
IDT_ABBR            equ 0x008E
IDT_SIZE            equ 256 * 8 - 1

VIDEO_ADDRESS       equ 0xB8000     ; 显存地址
VIDEO_COLS          equ 80          ; 列数
VIDEO_ROWS          equ 25          ; 行数

FLOPPY_NO           equ 0           ; 0~3 for floppy A~D
FLOPPY_NO_MASK      equ 0b0001      ; 要开启的软驱马达，按照二进制位 DCBA
FLOPPY_WAIT_DELAY   equ 500         ; 延时 500ms 保证电机启动
FLOPPY_HEAD_DELAY   equ 150         ; 延时 150ms 保证磁头到位
FLOPPY_RETRY_COUNT  equ 1000        ; 重复次数
FLOPPY_DCR_VALUE    equ 0x00        ; 500 Kbps
FLOPPY_IO_DOR       equ 0x03f2      ; Degital output register (WO)
FLOPPY_IO_STATUS    equ 0x03f4      ; Main status register (RO)
FLOPPY_IO_DATA      equ 0x03f5      ; Data register (R/W)
FLOPPY_IO_DIR       equ 0x03f7      ; Digital input register (RO)
FLOPPY_IO_DCR       equ 0x03f7      ; Diskette Control Register (WO) 控制传输速率

PIC1_CMD            equ 0x20        ; 主 PIC 命令端口 (ICW1/OCW2)
PIC1_DATA           equ 0x21        ; 主 PIC 数据端口 (ICW2/ICW3/ICW4/OCW1)
PIC2_CMD            equ 0xA0        ; 从 PIC 命令端口
PIC2_DATA           equ 0xA1        ; 从 PIC 数据端口

TIMER_PIO_FREQ      equ 1193180     ; PIT 输入为 1193180 Hz
TIMER_IRQ0_FREQ     equ 100         ; IRQ0 配置为 100 Hz
TIMER_COUNTER       equ TIMER_PIO_FREQ / TIMER_IRQ0_FREQ
TIMER_CFG_IO        equ 0x43        ; PIT 配置端口
TIMER_IO            equ 0x40        ; 计数器端口
TIMER_CW            equ 0x34        ; 控制字

RSVDSECCNT          equ 1           ; 保留扇区
FAT_SIZE            equ 9           ; FAT 表大小
FAT_COUNT           equ 2           ; FAT 表数量
ROOT_DIR_SIZE       equ 14          ; 根目录大小
ROOT_ENTRY_COUNT    equ 224         ; 根目录 Entry 数量
FAT_LBA_OFFSET      equ RSVDSECCNT + FAT_SIZE * FAT_COUNT + ROOT_DIR_SIZE - 2

VGA_IO_INDEX        equ 0x03D4      ; VGA 显卡的索引寄存器
VGA_IO_DATA         equ 0x03D5      ; VGA 显卡的数据寄存器
VGA_IDX_POS_HIGH    equ 0x0E        ; 光标位置高八位
VGA_IDX_POS_LOW     equ 0x0F        ; 光标位置低八位

BOOT_DEVICE         equ 0x00FFFFFF  ; 启动设备，用于 multiboot


[org 0x8000]
[bits 16]
start:
    xor ax, ax                      ; 设置段寄存器
    mov ss, ax
    mov ds, ax
    mov es, ax

    ; Stack
    mov sp, SP_ADDR

    ; 获取内存信息
    call detect_memory_rm
    jc .e820_err

    ; 进入 32 位模式
    cli                             ; 关闭中断

    ; 配置 GDT
    mov ax, GDT_ADDR >> 4
    mov ds, ax
    mov dword [ds:0], 0
    mov dword [ds:4], 0
    mov dword [ds:8], GDT_DATA_ENTRY & 0xFFFFFFFF
    mov dword [ds:12], GDT_DATA_ENTRY >> 32
    mov dword [ds:16], GDT_CODE_ENTRY & 0xFFFFFFFF
    mov dword [ds:20], GDT_CODE_ENTRY >> 32

    lgdt [cs: gdtr_val]             ; 使用 lgdt 设置 gdtr 寄存器

    ; 打开 A20
    in al, 0x92                     ; 使用 0x92 快速 A20
    test al, 0x02                   ; 如果第 1 位已经被设置
    jnz .a20_done                   ; 跳过设置
    or al, 0x02                     ; 没有的话，将第 1 位置1
    out 0x92, al                    ; 写回
.a20_done:

    ; 设置 PE
    mov eax, cr0                    ; 读取 cr0 到 eax
    or eax, 1                       ; 设置 PE 位
    mov cr0, eax                    ; 写回 cr0

    ; 跳转刷新流水线
    jmp GDT_CODE_SEG:pm_start       ; 使用 2# 代码段

.e820_err:
    mov si, err_E820_str
    call print_string_rm
.halt:
    hlt
    jmp .halt


; Print string pointed to by SI, in realmode
print_string_rm:
    push ax
.print_loop:
    lodsb                           ; Load byte at DS:SI into AL and increment SI
    cmp al, 0                       ; Check for null terminator
    je .done                        ; If null terminator, we're done
    mov ah, 0x0e                    ; BIOS teletype function
    int 0x10                        ; Call BIOS video interrupt
    jmp .print_loop                 ; Repeat for next character
.done:
    pop ax
    ret


; 获取内存信息, 使用 int 0x15, ax=0xE820 (我相信现在的 BIOS 都支持吧)
detect_memory_rm:
    ; 先清空目标内存
    mov di, MB_MEM_INFO_ADDR
    xor ax, ax
    mov cx, 0x200                   ; 清空 1kB 内存
    cld
    rep stosw

    mov [MB_MEM_INFO_ADDR - 4], 20

    ; 读取内存布局图
    xor bp, bp                      ; bp 用来计数
    mov di, MB_MEM_INFO_ADDR        ; di 放smap信息的地方
    mov eax, 0xE820                 ; 中断功能
    xor ebx, ebx                    ; 开始调用时 ebx 必须为 0
    mov ecx, 24                     ; 请求 24 字节
    mov edx, 0x534D4150             ; Magic Number 'SMAP'
    int 0x15                        ; 调用 15 号中断
    jc .faild
    mov edx, 0x534D4150
    cmp eax, edx
    jne .faild
    test ebx, ebx
    jz .faild
    jmp .jumpin
.continue:
    mov eax, 0xE820
    mov ecx, 24
    mov edx, 0x534D4150
    int 0x15
    jc .ok_e820
.jumpin:
    jcxz .next_entry
    cmp ecx, 20
    jb .faild
    cmp ecx, 24
    jbe .good
    jmp .faild
.good:
    mov ecx, [es:di + 8]
    or ecx, [es:di + 12]
    jz .next_entry
    mov [di - 4], 20
    inc bp
    add di, 24
.next_entry:
    test ebx, ebx
    jne .continue

.ok_e820:
    clc
    mov [mmap_size], bp
    ret

.faild:
    stc
    ret


[bits 32]
pm_start:
    mov ax, GDT_DATA_SEG            ; 数据段
    mov es, ax
    mov ds, ax
    mov ss, ax
    mov gs, ax
    mov fs, ax
    mov esp, SP_ADDR                ; 栈

    call init_pit
    call init_idt_pic

    sti

    call get_cursor_pos

    mov esi, hello_str
    call print_string

    call config_mbinfo              ; 获取 Multiboot Info
    jc halt

    call print_mmap                 ; 打印 mmap 信息
    mov esi, mem_lower_str          ; 打印 mem 信息
    call print_string
    mov eax, [MB_INFO_ADDR + 4]
    call print_u32_hex
    call put_lf
    mov esi, mem_upper_str
    call print_string
    mov eax, [MB_INFO_ADDR + 8]
    call print_u32_hex
    call put_lf

    call put_lf

    call init_floppy                ; 初始化软驱
    jc halt

    call read_fat                   ; 读取 FAT 表
    jc halt

    call read_root_directory        ; 读取根目录
    jc halt

    mov esi, loader_cfg_path_str    ; 加载 loader.cfg
    call find_file
    jc halt

    mov edi, CFG_FILE_ADDR
    mov ecx, CFG_FILE_SIZE
    call load_file
    jc halt

    call parse_cfg                  ; 解析
    jc halt

    call get_filename               ; 从 cmdline 获取文件名

    mov esi, found_boot_name_str    ; 打印信息
    call print_string
    mov esi, CFG_BOOT_NAME
    call print_string
    call put_lf
    mov esi, print_path_str
    call print_string
    mov esi, CFG_KERNEL_PATH
    call print_string
    call put_lf
    mov esi, print_cmdline_str
    call print_string
    mov esi, CFG_KERNEL_CMDLINE
    call print_string
    call put_lf
    call put_lf

    mov esi, CFG_KERNEL_PATH
    call find_file
    jc halt
    mov [kernel_size], edx

    mov edi, KERNEL_HEAD_ADDR
    mov ecx, KERNEL_HEAD_SIZE
    call load_file
    jc halt

    call check_multiboot
    jc halt
    call check_elf_head
    jc halt

    mov esi, ok_kernel_str
    call print_string

    call load_kernel
    jc halt

    cli
    mov eax, 0x1BADB002
    mov ebx, MB_INFO_ADDR
    jmp [KERNEL_HEAD_ADDR + 24] ; e_entry


; * FAT12 文件系统


; 读取 root 目录
read_root_directory:
    pushad
    mov eax, FAT_SIZE * FAT_COUNT + RSVDSECCNT
    mov ecx, ROOT_DIR_SIZE
    mov edi, ROOT_ADDR
.read:
    call floppy_read
    jc .err
    inc ax
    add edi, 512
    loop .read
    popad
    clc
    ret
.err:
    mov esi, read_root_err_str
    call print_string
    popad
    stc
    ret


; 读取 FAT 表
read_fat:
    pushad
    mov eax, RSVDSECCNT
    mov ecx, FAT_SIZE
    mov edi, FAT_ADDR
.read:
    call floppy_read
    jc .err
    inc ax
    add edi, 512
    loop .read
    popad
    clc
    ret
.err:
    mov esi, read_fat_err_str
    call print_string
    popad
    stc
    ret


; 查找一个文件
; ESI 文件路径 例如 'path/to/your/file', 0
; 自动大写，从根目录开始查找
; 返回 AX 起始 FAT 项
;     EDX 文件大小
find_file:
    push esi
    push edi
    push ebx
    push ecx
    sub esp, 16

    mov edi, esp
    call get_one_name
    jc .err
    mov ebx, esi
    mov esi, esp
    call find_entry_root
    jc .err
.find_next:
    cmp [ebx], 0
    je .ok

    inc ebx
    mov esi, ebx
    mov edi, esp
    call get_one_name
    jc .err
    mov ebx, esi
    mov esi, esp
    call find_subdir
    jc .err
    jmp .find_next
.ok:
    clc
.return:
    add esp, 16
    pop ecx
    pop ebx
    pop edi
    pop esi
    ret
.err:
    add esp, 16
    pop ecx
    pop ebx
    pop edi
    pop esi
    stc
    ret


; 加载一个文件
; AX 起始 FAT 缓冲区
; EDI 加载缓冲区
; ECX 缓冲区大小（以字节为单位）
load_file:
    pushad
.next_sector:
    cmp ecx, 512
    jb .ok
    add ax, FAT_LBA_OFFSET
    call floppy_read
    jc .err
    sub ax, FAT_LBA_OFFSET
    call get_next_fat_entry
    test ax, 0xFFFE
    jz .ok
    cmp ax, 0x0FF0
    jae .ok
    add edi, 512
    sub ecx, 512
    jmp .next_sector
.ok:
    popad
    clc
    ret
.err:
    mov esi, cannot_loadfile_str
    call print_string

    popad
    stc
    ret


; 获取从 ESI 开始，直到 / 或 0 的 11 字符文件名/目录名，自动大写
; EDI 目标缓冲区
; 返回时，ESI 指向 / 或者 0
get_one_name:
    push eax
    push ebx
    push ecx
    push edi

    mov al, 0x20                    ; 用空格清空缓冲区
    mov ebx, edi
    mov ecx, 11
    rep stosb
    mov edi, ebx

    mov ecx, 11
    mov ebx, esi
.next_char:
    mov al, [esi]                   ; AL -> 源字符
    test al, al                     ; AL == 0 ?
    jz .eon                         ; End of name
    cmp al, 0x2F                    ; AL == '/' ?
    je .eon                         ; End of name
    cmp al, 0x2e                    ; AL == '.' ?
    jne .no_dot
    cmp ecx, 3                      ; 是否已经超过了 8 字符的限制
    jb .invaild_name
    add edi, ecx                    ; edi 指向缓冲区末尾
    sub edi, 3                      ; edi 指向后缀名开头
    inc esi                         ; 下一个源字符
    mov ecx, 3                      ; 还剩 3 个字符
    jmp .next_char
.no_dot:
    jcxz .invaild_name              ; 是否到达缓冲区末尾？
    cmp al, 0x61                    ; 大于 0x61 'a'
    jb .skip_upper
    cmp al, 0x7A                    ; 小于 0x7A 'A'
    ja .skip_upper
    sub al, 0x20                    ; 转换为大写
.skip_upper:
    mov [edi], al                   ; 复制
    inc edi                         ; 增加 EDI, ESI，更新计数
    inc esi
    dec ecx
    jmp .next_char
.invaild_name:                      ; 溢出
    mov esi, invaild_name_str       ; 先打印错误信息
    call print_string
    mov esi, ebx                    ; 再打印 原ESI
    call print_string
    call put_lf                     ; 换行
    pop edi
    pop ecx
    pop ebx
    pop eax
    stc
    ret
.eon:
    pop edi
    pop ecx
    pop ebx
    pop eax
    clc
    ret


; 在 ROOT 目录查找一个 文件/目录
; ESI 文件名（11字符）
; 返回 AX 起始 FAT 项
;    EDX 文件大小
find_entry_root:
    push ebx
    push ecx
    push edi
    push ebp

    cld
    mov eax, ROOT_ENTRY_COUNT       ; eax 存储 ENTRY 数量
    mov ebp, esi
    mov edi, ROOT_ADDR
.cmp_loop:
    mov ebx, edi
    mov ecx, 11
    repe cmpsb
    je .found
    mov esi, ebp
    add edi, 32 - 11
    dec eax
    test eax, eax
    jnz .cmp_loop

    mov esi, nofind_ent_root_str
    call print_string
    mov ah, 0x07
    mov ecx, 11
.print_fn:
    mov al, [ebp]
    call put_char
    inc ebp
    loop .print_fn
    call put_lf
    stc
    jmp .return
.found:
    mov ax, [ebx + 26]              ; FAT 起始项在偏移 26 字节的位置
    mov edx, [ebx + 28]
    clc
.return:
    mov esi, ebp
    pop ebp
    pop edi
    pop ecx
    pop ebx
    ret


; 在子目录查找一个文件
; ax 子目录开始 FAT 项
; esi 文件名
; 返回 ax FAT 项序号
;     edx 文件大小
find_subdir:
    push ebx
    push ecx
    push ebp
    push esi
    push edi

    cld
    mov ebp, esi
.next_sector:
    mov edi, DIR_BUFFER_ADDR
    add ax, FAT_LBA_OFFSET
    call floppy_read
    jc .err

    mov edx, 16                     ; 一个扇区有 16 个 entry
    mov ebx, edi
.next_entry:
    mov ecx, 11
    repe cmpsb
    je .found
    add ebx, 32
    mov edi, ebx
    mov esi, ebp
    dec edx
    jnz .next_entry

    sub ax, FAT_LBA_OFFSET
    call get_next_fat_entry
    test ax, 0xFFFE
    jz .err
    cmp ax, 0x0FF0
    jb .next_sector
.err:
    mov esi, nofind_ent_sub_str
    call print_string
    mov ah, 0x07
    mov ecx, 11
.print_fn:
    mov al, [ebp]
    call put_char
    inc ebp
    loop .print_fn
    call put_lf

    xor ax, ax
    pop edi
    pop esi
    pop ebp
    pop ecx
    pop ebx
    stc
    ret
.found:
    mov ax, [ebx + 26]
    mov edx, [ebx + 28]
    pop edi
    pop esi
    pop ebp
    pop ecx
    pop ebx
    clc
    ret


; 获取下一个 FAT 项
; 输入 AX 上一个 FAT 项
; 输出 AX 下一个 FAT 项
get_next_fat_entry:
    push ebx
    push ecx
    push edx

    mov bx, 3                       ; bx 为系数 3
    mul bx                          ; dx:ax = ax * bx = ax * 3, 忽略 dx
    mov cx, ax                      ; cx 存储奇偶
    and cx, 1                       ; cx &= 0x01
    shr ax, 1                       ; ax = ax / 2 = 原ax * 1.5，即 FAT 表内的偏移

    xor ebx, ebx
    mov bx, ax
    mov ax, [ebx + FAT_ADDR]
    jcxz .even

    shr ax, 4
    jmp .return
.even:
    and ax, 0x0FFF
.return:
    pop edx
    pop ecx
    pop ebx
    ret


; * 软驱驱动


; 初始化软驱
init_floppy:
    pushad

    ; 启用马达并且选择软盘驱动器
    mov al, FLOPPY_NO_MASK << 4 | 4 | 8 | FLOPPY_NO
    mov dx, FLOPPY_IO_DOR
    out dx, al

    ; 设置速率
    mov al, FLOPPY_DCR_VALUE
    mov dx, FLOPPY_IO_DCR
    out dx, al

    mov ecx, FLOPPY_WAIT_DELAY * TIMER_IRQ0_FREQ / 1000
    call wait_ecx

    mov al, 0x03                    ; SECIFY
    call output_floppy
    jc .err

    mov al, 0xCF                    ; spec1
    call output_floppy
    jc .err

    mov al, 0x06                    ; Head load time 6 * 4 ms
    call output_floppy
    jc .err

    popad
    clc
    ret
.err:
    mov esi, init_floppy_err_str
    call print_string
    popad
    stc
    ret


; 从软盘读取一个扇区，使用 LBA
; AX 要读取的 LBA 号
; EDI 要读取到的地址
floppy_read:
    pushad

    ; LBA -> CHS
    mov bl, 18                      ; 每磁道有18个扇区
    div bl                          ; AX / BL, ah 为余数（扇区号）, al 为商
    inc ah                          ; CHS 的扇区号从 1 开始，所以加一
    mov bh, ah                      ; bh 为扇区号
    mov dl, al                      ; dl 为磁头号
    and dl, 1                       ; dl = 0 / 1
    mov bl, al                      ; bl 为磁道号
    shr bl, 1                       ; bl /= 2

    mov ecx, 5                      ; 最多尝试五次
.fd_seek:
    call floppy_seek
    jnc .setup_dma

    call floppy_recalibrate
    jc .err_seek
    loop .fd_seek
    jmp .err_seek
.setup_dma:
    pushfd
    cli                             ; 禁用中断

    mov al, 0x06
    out 0x0A, al                    ; 禁用 DMA 2
    times 3 nop

    mov al, 0x46
    out 0x0C, al                    ; 写入命令
    times 3 nop

    out 0x0B, al                    ; 写入命令x2
    times 3 nop

    mov eax, edi
    out 0x04, al                    ; bits 0-7
    times 3 nop

    shr eax, 8
    out 0x04, al                    ; bits 8-15
    times 2 nop

    shr eax, 8
    out 0x81, al                    ; bits 16-19
    times 3 nop

    mov al, 0xFF                    ; low 8 bits of count - 1
    out 0x05, al
    times 3 nop

    mov al, 0x01                    ; high 8 bits of count - 1
    out 0x05, al
    times 3 nop

    mov al, 0x02                    ; 启用 DMA 2
    out 0x0A, al

    popfd                           ; 恢复中断

    ; 开始发送读取命令
    mov [floppy_read_count], 7      ; 7 byte 结果
    mov al, 0xE6                    ; 读取命令
    call output_floppy
    jc .err_read

    mov al, dl                      ; 驱动器号 + 磁头
    shl al, 2
    or al, FLOPPY_NO
    call output_floppy
    jc .err_read

    mov al, bl                      ; 磁道号
    call output_floppy
    jc .err_read
    
    mov al, dl                      ; 磁头号
    call output_floppy
    jc .err_read

    mov al, bh                      ; 扇区号
    call output_floppy
    jc .err_read

    mov al, 2                       ; 扇区大小，2 => 512 字节
    call output_floppy
    jc .err_read

    mov al, 18                      ; EOT
    call output_floppy
    jc .err_read

    mov al, 0x1B                    ; 扇区之间间隔长度，这里是 0x1B
    call output_floppy
    jc .err_read

    mov al, 0xFF                    ; 同样是扇区大小，但是一般设为 0xFF
    call output_floppy              ; 因为只在上面那个(N)等于 0 时才有用
    jc .err_read

    call wait_floppy_int
    jc .err_read
    mov al, [floppy_result0]
    and al, 0xC0
    cmp al, 0x00
    jne .err_read

    mov [floppy_read_count], 0

    popad
    clc
    ret
.err_read:
    mov esi, floppy_read_err
    call print_string
    popad
    stc
    ret
.err_seek:
    mov esi, floppy_seek_err
    call print_string
.err:
    popad
    stc
    ret


; 移动磁头到某个磁道
; bl 磁道号
; dl 磁头号 0 / 1
floppy_seek:
    pusha

    cmp bl, [floppy_cur_track]
    je .ok

    mov [floppy_read_count], 0

    mov cl, bl
    and cl, 1
    shl cl, 2
    or cl, FLOPPY_NO

    mov al, 0x0F
    call output_floppy
    jc .err

    mov al, cl
    call output_floppy
    jc .err

    mov al, bl
    call output_floppy
    jc .err

    mov ecx, FLOPPY_HEAD_DELAY * TIMER_IRQ0_FREQ / 1000
    call wait_ecx

    call wait_floppy_int
    jc .err
    mov al, [floppy_result0]
    and al, 0xE0
    cmp al, 0x20
    jne .err

    mov [floppy_cur_track], bl
.ok:
    popa
    clc
    ret
.err:
    popa
    stc
    ret


; 重新校正软驱
floppy_recalibrate:
    pusha

    mov [floppy_read_count], 0

    mov al, 0x07
    call output_floppy
    jc .err

    mov al, FLOPPY_NO
    call output_floppy
    jc .err

    mov ecx, FLOPPY_HEAD_DELAY * TIMER_IRQ0_FREQ / 1000
    call wait_ecx

    call wait_floppy_int
    jc .err
    mov al, [floppy_result0]
    and al, 0xE0
    cmp al, 0x20
    jne .err

    ; Ok
    mov [floppy_cur_track], 0

    popa
    clc
    ret
.err:
    mov esi, floppy_reca_err_str
    call print_string
    stc
    popa
    ret


; 输出到软驱 FLOPPY_IO_DATA
; al 输出的数据
output_floppy:
    pushad
    call wait_floppy
    jc .err
    mov dx, FLOPPY_IO_DATA
    out dx, al
    popad
    clc
    ret
.err:
    popad
    stc
    ret


; 等待软驱的中断 IRQ6
wait_floppy_int:
    pushad
    mov ecx, FLOPPY_RETRY_COUNT
.wait_loop:
    mov eax, [floppy_flag]
    and eax, 1
    jnz .ok
    hlt
    loop .wait_loop
    popad
    stc
    ret
.ok:
    mov [floppy_flag], 0
    popad
    clc
    ret


; 等待软驱写就绪
wait_floppy:
    pushad
    mov ecx, FLOPPY_RETRY_COUNT
.read_msr:
    mov dx, FLOPPY_IO_STATUS
    in al, dx
    and al, 0b11000000              ; 读取高 2 位
    cmp al, 0b10000000              ; 写就绪
    je .ok
    mov dx, FLOPPY_IO_DATA
    ; in al, dx
    
    times 10 nop
    loop .read_msr
    popad
    stc
    ret
.ok:
    popad
    clc
    ret


; * 显示


; 清空屏幕
clean_screen:
    pushad
    mov edi, VIDEO_ADDRESS
    mov ecx, VIDEO_ROWS * VIDEO_COLS
    xor eax, eax
    cld
    rep stosw
    popad
    ret

print_string:
    push eax
.print_loop:
    lodsb                           ; Load byte at DS:ESI into AL and increment ESI
    test al, al                     ; Check for null terminator
    je .done                        ; If null terminator, we're done
    mov ah, 0x07                    ; Default, gray
    call put_char                   ; Put char
    jmp .print_loop                 ; Repeat for next character
.done:
    pop eax
    ret


; 打印 MMAP
print_mmap:
    pushad

    mov esi, mmap_head_str
    call print_string

    mov ecx, [mmap_size]
    mov edi, MB_MEM_INFO_ADDR
    xor ebp, ebp                    ; 使用 ebp 计数
.print_loop:
    mov eax, ebp                    ; No.
    call print_u16_hex              ; Print u16

    call put_space

    mov eax, [edi + 0]              ; Start Address LOW
    mov edx, [edi + 4]              ; Start Address HIGH
    call print_u64_hex              ; Print u64

    call put_space                  ; Space

    add eax, [edi + 8]              ; End Address LOW
    adc edx, [edi + 12]             ; End Address HIGH (带进位加法)
    call print_u64_hex              ; Print u64

    call put_space                  ; Space

    mov eax, [edi + 8]              ; Length LOW
    mov edx, [edi + 12]             ; Length HIGH
    call print_u64_hex              ; Print u64

    call put_space                  ; Space

    mov eax, [edi + 16]             ; Type
    call print_u32_hex              ; Print u32

    call put_space                  ; Space

    mov ebx, eax                    ; 移动 eax 到 ebx
    cmp ebx, 8                      ; 是否小于 8
    jb .search_table                ; 小于 8, 直接查表
    xor ebx, ebx                    ; 大于 8, 为 Unkonwn ( 0 号)
.search_table:
    mov esi, [ebx * 4 + .type_table]

    call print_string

    call put_lf

    inc ebp                         ; 增加计数
    add edi, 24                     ; 下一个
    loop .print_loop

    call put_lf

    popad
    ret
.type_table:
    dd unknown_str
    dd available_str
    dd reserved_str
    dd acpi_reclaim_str
    dd acpi_nvs_str
    dd defective_mem_str
    dd unknown_str
    dd unknown_str


; 打印一个空格
put_space:
    push eax
    mov ax, 0x0020                  ; Space
    call put_char
    pop eax
    ret


; 打印一个换行
put_lf:
    push eax
    mov ax, 0x000A                  ; LF
    call put_char
    pop eax
    ret


; 输出一个字符
; AL: 字符 ASCII 码
; AH: 字符属性 (常见值 0x07)
put_char:
    pushad

    mov edx, eax                    ; 把字符和属性存储到 DX
    mov eax, [cursor_col]           ; eax 表示光标COL（列）
    mov ecx, [cursor_row]           ; ecx 表示光标ROW（行）

    cmp dl, 0x0A                    ; \n
    je .new_line
    cmp dl, 0x0D                    ; \r
    je .r_char
    cmp dl, 0x09                    ; \t
    je .t_char

    call set_cursor_char_dx         ; 设置显存
    inc eax                         ; 增加 col

.check_new_line:
    cmp eax, VIDEO_COLS
    jb .return                      ; 需要换行
.new_line:
    inc ecx                         ; row++
.r_char:
    xor eax, eax                    ; col=0
    cmp ecx, VIDEO_ROWS
    jae .need_scroll                ; 需要滚屏
    jmp .return
.need_scroll:
    call scroll_screen
    mov ecx, VIDEO_ROWS - 1
.return:
    mov [cursor_col], eax
    mov [cursor_row], ecx
    call set_cursor_pos
    popad
    ret
.t_char:
    add eax, 8
    and eax, 0xFFFFFFF8
    jmp .check_new_line


; 设置光标所在的字符
; DX:需要设置的字符和属性
set_cursor_char_dx:
    pushad
    mov ecx, edx
    mov eax, [cursor_row]
    mov ebx, VIDEO_COLS
    mul ebx
    add eax, [cursor_col]
    shl eax, 1
    mov ebx, eax
    mov [ebx + VIDEO_ADDRESS], cx
    popad
    ret


; 滚动屏幕 1 行，无输入
scroll_screen:
    pushad

    mov edi, VIDEO_ADDRESS
    mov esi, VIDEO_ADDRESS + VIDEO_COLS * 2
    mov ecx, (VIDEO_ROWS - 1) * VIDEO_COLS
.scroll_loop:
    mov ax, [esi]
    mov [edi], ax
    add edi, 2
    add esi, 2
    loop .scroll_loop

    mov edi, VIDEO_ADDRESS + VIDEO_COLS * (VIDEO_ROWS - 1) * 2
    mov ecx, VIDEO_COLS
.clear_loop:
    mov word [edi], 0
    add edi, 2
    loop .clear_loop

    popad
    ret


; 打印 64 位无符号数字，输出 16 进制
; eax: 数字低位
; edx: 数字高位
print_u64_hex:
    xchg eax, edx
    call print_u32_hex
    xchg eax, edx
    call print_u32_hex
    ret


; 打印 32 位无符号数字，输出 16 进制
; eax: 数字
print_u32_hex:
    pushad

    mov edx, eax
    mov ah, 0x07
    mov ecx, 8
.print_loop:
    mov ebx, edx
    shr ebx, 28
    mov al, [ebx + hex_upper_str]
    call put_char
    shl edx, 4
    loop .print_loop

    popad
    ret


; 打印 16 位无符号 16 进制数字
; ax: 要打印的数字
print_u16_hex:
    pushad

    xor ebx, ebx
    mov edx, eax
    mov ah, 0x07
    mov ecx, 4
.print_loop:
    mov bx, dx
    shr ebx, 12
    mov al, [ebx + hex_upper_str]
    call put_char
    shl dx, 4
    loop .print_loop

    popad
    ret

; 打印 8 位无符号 16 进制数字
; al: 要打印的数字
print_u8_hex:
    pushad
    mov cl, al
    mov ah, 0x07
    movzx ebx, al
    shr ebx, 4
    mov al, [ebx + hex_upper_str]
    call put_char
    mov bl, cl
    and ebx, 0x0F
    mov al, [ebx + hex_upper_str]
    call put_char
    popad
    ret


; 获取 VGA 显卡的光标位置，存储在 cursor_col 和 cursor_row
get_cursor_pos:
    pushad

    mov dx, VGA_IO_INDEX            ; 读取高位
    mov al, VGA_IDX_POS_HIGH
    out dx, al
    mov dx, VGA_IO_DATA
    in al, dx
    mov ah, al                      ; 高位在 ah

    mov dx, VGA_IO_INDEX            ; 读取低位
    mov al, VGA_IDX_POS_LOW
    out dx, al
    mov dx, VGA_IO_DATA
    in al, dx                       ; 低位在 al

    mov bl, VIDEO_COLS
    div bl                          ; ax / bl，al 为 row，ah 为 col

    movzx edx, al
    mov [cursor_row], edx
    movzx edx, ah
    mov [cursor_col], edx

    popad
    ret


; 设置光标位置到 cursor_row 和 cursor_col
set_cursor_pos:
    pushad

    mov eax, [cursor_row]           ; 获取光标位置 0 ~ 1999
    mov ecx, VIDEO_COLS
    mul ecx
    add eax, [cursor_col]
    mov ecx, eax

    mov dx, VGA_IO_INDEX            ; 写入低位
    mov al, VGA_IDX_POS_LOW
    out dx, al
    mov dx, VGA_IO_DATA
    mov al, cl
    out dx, al

    mov dx, VGA_IO_INDEX            ; 写入高位
    mov al, VGA_IDX_POS_HIGH
    out dx, al
    mov dx, VGA_IO_DATA
    mov al, ch
    out dx, al

    popad
    ret


; IDT, PIC & PIT


; 初始化 IDT 和 PIC
init_idt_pic:
    pushad

    ; Fill up the idt
    mov  ecx, 256
    mov  esi, null_handler
    call fill_all_idt

    mov esi, floppy_interrupt_handler
    mov ebx, 0x26                  ; 中断向量号
    call set_idt_gate

    mov esi, timer_intrrupt_handler
    mov ebx, 0x20                  ; 中断向量号
    call set_idt_gate

    lidt [idtr_val]

    ; 初始化主 PIC (ICW1: 边沿触发, 级联)
    mov al, 0x11
    out PIC1_CMD, al
    mov al, 0x20
    out PIC1_DATA, al
    mov al, 0x04                   ; 二进制 0000 0100, 表示 IRQ2 接从片
    out PIC1_DATA, al
    mov al, 0x01
    out PIC1_DATA, al

    mov al, 0x11
    out PIC2_CMD, al
    mov al, 0x28
    out PIC2_DATA, al
    mov al, 0x02                   ; 二进制 0000 0010
    out PIC2_DATA, al
    mov al, 0x01
    out PIC2_DATA, al

    ; 屏蔽所有中断 (OCW1), 暂时全部禁止, 后面再单独开启
    mov al, 0xFF
    out PIC1_DATA, al
    nop
    out PIC2_DATA, al

    ; 打开中断
    ; 主 PIC
    mov al, 0xBE
    out PIC1_DATA, al
    ; 从 PIC
    mov al, 0xFF
    out PIC2_DATA, al

    popad
    ret


; 填充所有 IDT 项
fill_all_idt:
    pushad
    xor ebx, ebx
.set_loop:
    call set_idt_gate
    inc ebx
    loop .set_loop
    popad
    ret


; 设置一个 IDT 项
set_idt_gate:
    pushad
    lea ebx, [ebx * 8 + IDT_ADDR]

    mov word[ebx], si
    shr esi, 16
    mov word [ebx + 6], si

    mov word [ebx + 2], GDT_CODE_SEG
    mov word [ebx + 5], IDT_ABBR
    popad
    ret


; 初始化 PIT
init_pit:
    pushad
    mov dx, TIMER_CFG_IO
    mov al, TIMER_CW
    out dx, al

    mov dx, TIMER_IO
    mov al, TIMER_COUNTER & 0x00FF
    out dx, al

    mov al, TIMER_COUNTER >> 8
    out dx, al
    popad
    ret


; 等待 ECX 指定的 tick 数
wait_ecx:
    add ecx, [tick]
.loop_wait:
    times 8 nop
    cmp ecx, [tick]
    jb .loop_wait
    ret


; * 中断程序


align 4
floppy_interrupt_handler:
    pushad

    ; Read result
    mov ebx, floppy_result_start
    mov dx, FLOPPY_IO_DATA
    mov ecx, [floppy_read_count]
    test ecx, ecx
    jnz .readio                     ; 不 0 的话，直接读取
    mov al, 0x08                    ; 为 0，使用FD_SENSEI
    call output_floppy              ; 输出指令
    mov ecx, 2                      ; 2 个结果
.readio:
    in al, dx
    mov [ebx], al
    inc ebx
    cmp ebx, floppy_result_end
    je .err
    loop .readio
    mov [floppy_read_count], 0

    or dword [floppy_flag], 1

    mov al, 0x20
    out PIC1_CMD, al
    popad
    iret
.err:
    mov esi, floppy_too_many_str
    call print_string
    jmp halt


align 4
timer_intrrupt_handler:
    pushad

    inc dword [tick]

    mov al, 0x20
    out PIC1_CMD, al
    popad
    iret


align 4
null_handler:
    mov esi, null_handler_str
    call print_string
    jmp halt


; * MISC


; 获取 Multiboot 的 MEM 信息，使用 MMAP 信息, 同时设置 MMAP 和 MEM 的 Multiboot Info 条目
get_mem_info:   
    pushad

    xor ebp, ebp                    ; 高端内存大小
    mov ecx, [mmap_size]
    mov esi, MB_MEM_INFO_ADDR
.read_lower:
    cmp dword [esi + 16], 1         ; Type = Available
    jne .next_lower

    test dword [esi], 0xFFFFFFFF
    jnz .next_lower

    test dword [esi + 4], 0xFFFFFFFF ; 仅低 4GB 内存
    jnz .next_lower

    mov eax, [esi + 8]              ; 最大为 640 KB
    cmp eax, 0xA0000
    jbe .set_lower
    mov eax, 0xA0000
.set_lower:
    shr eax, 10
    mov [MB_INFO_ADDR + 4], eax
    jmp .find_upper_avi
.next_lower:
    add esi, 24
    loop .read_lower
    jmp .err
.find_upper_avi:
    add esi, 24
    cmp dword [esi + 16], 1
    je .find_upper_noavi
    loop .find_upper_avi
    jmp .err
.find_upper_noavi:
    add esi, 24
    cmp dword [esi + 16], 1
    jne .found_noavi
    loop .find_upper_noavi
.found_noavi:   
    sub esi, 24
    mov eax, [esi]
    mov edx, [esi + 4]
    add eax, [esi + 8]
    adc edx, [esi + 12]

    sub eax, 0x100000               ; 减去 1MB，得到从 1MB 起始的字节数
    sbb edx, 0

    shrd eax, edx, 10               ; 以 KB 为单位
    mov dword [MB_INFO_ADDR + 8], eax
    or dword [MB_INFO_ADDR], 0x01 + 0x40

    ; 写入 MMAP 信息
    mov eax, [mmap_size]
    mov ecx, 24
    mul ecx
    mov dword [MB_INFO_ADDR + 44], eax
    mov dword [MB_INFO_ADDR + 48], MB_MEM_INFO_ADDR - 4

    popad
    clc
    ret
.err:
    popad
    stc
    ret


; 配置 Multiboot Info
; 参考 https://www.gnu.org/software/grub/manual/multiboot/html_node/Boot-information-format.html
config_mbinfo:  
    pushad
    mov dword [MB_INFO_ADDR], 0x0206
    mov dword [MB_INFO_ADDR + 12], BOOT_DEVICE  ; boot_device
    mov dword [MB_INFO_ADDR + 16], CFG_KERNEL_CMDLINE ; cmdline
    mov dword [MB_INFO_ADDR + 64], bootloader_name_str ; boot_loader_name
    call get_mem_info
    popad
    ret


; 比较字符是否一样
; 输入 ESI 和 EDI
; CF 表示不同，cf 表示相同
cmp_str:
    pushad
    cld
    cmpsb
    jne .diff
.cmp_loop:
    cmp [esi - 1], 0
    je .same
    cmpsb
    je .cmp_loop
.diff:
    popad
    stc
    ret
.same:
    popad
    clc
    ret


; 解析内存中的 CFG 文件
; 输入 EDX 文件大小
parse_cfg:
    pushad
    xor eax, eax
    mov ecx, CFG_PARSE_BUF_SIZE + 1
    mov edi, CFG_PARSE_KEY_ADDR
    rep stosb
    mov ecx, CFG_PARSE_BUF_SIZE + 1
    mov edi, CFG_BOOT_NAME
    rep stosb
    mov ecx, CFG_PARSE_BUF_SIZE + 1
    mov edi, CFG_KERNEL_CMDLINE
    rep stosb
    mov esi, CFG_FILE_ADDR
.read_line:
    mov al, [esi]
    cmp al, 0
    je .eof
    cmp al, 0x20                    ; 空格
    je .next_char
    cmp al, 0x09                    ; \t
    je .next_char
    cmp al, 0x0D                    ; \r
    je .next_char
    cmp al, 0x0A                    ; \n
    je .next_char
    cmp al, 0x23                    ; #
    je .comment

    mov edi, CFG_PARSE_KEY_ADDR
    mov ecx, CFG_PARSE_BUF_SIZE
.key_nextc:
    mov [edi], al                   ; 复制当前的字符
    inc esi                         ; 增加寄存器
    inc edi
    dec edx                         ; 检查是否到达文件末尾
    jz .eof
    mov al, [esi]                   ; 检查下一个字符
    cmp al, 0x3D                    ; 为 =
    je .parse_key                   ; key 读取完毕
    cmp al, 0x0A                    ; 为 \n
    je .invaild_key                 ; 非法 key
    cmp al, 0x0D                    ; 为 \r
    je .invaild_key                 ; 非法 key
    loop .key_nextc                 ; 继续复制 key
    mov esi, too_long_key_str       ; 到达缓冲区末尾
    call print_string               ; 报错退出
    popad
    stc
    ret
.parse_key:
    mov [edi], 0                    ; 确保 \0 结尾
    mov ebx, esi
    inc ebx
    dec edx
    jz .eof
    cmp [ebx], 0
    je .eof

    mov esi, CFG_PARSE_KEY_ADDR
    mov edi, bootname_str
    call cmp_str
    jnc .bootname
    mov edi, kernel_cmdline_str
    call cmp_str
    jnc .kernel_cmdline

    mov esi, unknown_key_err_str
    call print_string
    mov esi, CFG_PARSE_KEY_ADDR
    call print_string
    call put_lf
    popad
    stc
    ret
.bootname:
    mov edi, CFG_BOOT_NAME
    jmp .parse_val
.kernel_cmdline:
    mov edi, CFG_KERNEL_CMDLINE
.parse_val:
    mov ecx, CFG_PARSE_BUF_SIZE
    mov esi, ebx
.parse_next_val:
    mov al, [esi]
    cmp al, 0
    je .eof
    cmp al, 0x0A                    ; \n
    je .next_char
    cmp al, 0x0D                    ; \r
    je .next_char
    cmp al, 0x23                    ; #
    je .read_line
    mov [edi], al
    inc edi
    inc esi
    dec edx
    jz .eof
    loop .parse_next_val
    
    mov esi, too_long_val_str
    call print_string
    mov esi, CFG_PARSE_KEY_ADDR
    call print_string
    call put_lf
    popad
    stc
    ret
.next_char:
    inc esi
    dec edx
    jz .eof
    jmp .read_line
.comment:
    inc esi
    dec edx
    jz .eof
    cmp [esi], 0x0A                 ; \n
    jne .comment
    jmp .next_char
.eof:
    popad
    clc
    ret
.invaild_key:
    mov esi, invaild_key_str
    call print_string
    mov esi, CFG_PARSE_KEY_ADDR
    call print_string
    call put_lf
    popad
    stc
    ret

; 从 CFG_KERNEL_CMDLINE 获取 kernel path
; 放在 CFG_KERNEL_PATH
get_filename:
    pushad
    xor eax, eax                        ; 清空缓冲区
    mov ecx, CFG_PARSE_BUF_SIZE + 1     ; 多清除一个作为末尾的 0
    mov edi, CFG_KERNEL_PATH
    rep stosb

    mov esi, CFG_KERNEL_CMDLINE
    mov edi, CFG_KERNEL_PATH
.copy_next:
    mov al, [esi]
    cmp al, 0
    je .ok
    cmp al, 0x20
    je .ok
    mov [edi], al
    inc esi
    inc edi
    jmp .copy_next
.ok:
    popad
    ret


; 检查 Multiboot 标志
check_multiboot:
    pushad

    mov esi, KERNEL_HEAD_ADDR
    mov ecx, KERNEL_HEAD_SIZE / 4
    mov eax, 0x1BADB002
.check_next:
    cmp [esi], eax
    je .found
    add esi, 4
    loop .check_next
    mov esi, invaild_kernel_str
    call print_string
    popad
    stc
    ret
.found:
    mov edx, [esi + 4]
    add eax, edx
    add eax, [esi + 8]
    jnz .err_checksum
    mov [mboot_flags], edx
    popad
    clc
    ret
.err_checksum:
    mov esi, err_check_sum_str
    call print_string
    popad
    stc
    ret


; 读取内核 ELF 头
check_elf_head:
    pushad
    mov eax, [KERNEL_HEAD_ADDR + 0]
    cmp eax, 0x464C457F             ; 0x7F, 'ELF'
    jne .invaild_head
    mov eax, [KERNEL_HEAD_ADDR + 4]
    and eax, 0x00FFFFFF
    cmp eax, 0x00010101             ; 32 bits + LSB + current version
    jne .invaild_head
    mov eax, [KERNEL_HEAD_ADDR + 16]
    cmp eax, 0x00030002             ; EM_386 + ET_EXEC
    jne .invaild_head
    mov ecx, [KERNEL_HEAD_ADDR + 28]
    jecxz .invaild_head             ; e_phoff
    mov ax, [KERNEL_HEAD_ADDR + 42]
    cmp ax, 0x20                    ; e_phentsize
    jne .invaild_head
    movzx eax, word [KERNEL_HEAD_ADDR + 44] ; e_phnum
    shl eax, 5
    add eax, ecx
    cmp eax, KERNEL_HEAD_SIZE
    ja .invaild_head

    popad
    clc
    ret
.invaild_head:
    mov esi, invaild_elfhead_str
    call print_string
    popad
    stc
    ret


; 加载内核
; ax 内核文件开始 FAT 项
load_kernel:
    pushad

    mov ebx, [KERNEL_HEAD_ADDR + 28]
    add ebx, KERNEL_HEAD_ADDR
    movzx ecx, word [KERNEL_HEAD_ADDR + 44] ; e_phnum
    jecxz .ok

.load_loop:
    cmp dword [ebx], 1              ; PT_LOAD
    jne .next_entry
    mov edx, [ebx + 16]             ; p_filesz
    test edx, edx
    jz .next_entry

    mov esi, [ebx + 4]              ; p_offset
    mov edi, [ebx + 12]             ; p_paddr
    call load_kernel_seg
    jc .err
.next_entry:
    add ebx, 0x20
    loop .load_loop
.ok:
    popad
    clc
    ret
.err:
    mov esi, cannot_loadker_str
    popad
    stc
    ret


; 加载内核文件的某一段到内存中
; 输入 AX 内核文件起始 FAT 序号
;     ESI 文件内起始地址
;     EDI 要加载到的内存位置
;     EDX 加载字节数
load_kernel_seg:
    pushad

    mov ecx, esi                    ; 检查是否超出大小
    add ecx, edx
    cmp [kernel_size], ecx
    jb .err

    mov ecx, esi
    shr ecx, 9
    jz .skip_fat_loop               ; ecx 为 0, 在文件开头 1 扇区
.fat_loop:
    call get_next_fat_entry         ; 找到要读取的第一个扇区
    test ax, 0xFFFE                 ; 必须 0x001 < ax < 0xFF0
    jz .err
    cmp ax, 0xFF0
    jae .err
    loop .fat_loop
.skip_fat_loop:
    and esi, 0x01FF
    mov ebp, esi                    ; ebp 代表此扇区内数据的开始位置
    mov ebx, edi                    ; ebx 代表数据的目标地址
.read_next_sector:
    test ax, 0xFFFE                 ; 检查 fat 项
    jz .ok
    cmp ax, 0xFF0
    jae .ok

    add ax, FAT_LBA_OFFSET          ; 读取到缓冲区
    mov edi, KERNEL_BUF_ADDR
    call floppy_read
    jc .err

    sub ax, FAT_LBA_OFFSET          ; 获取下一个
    call get_next_fat_entry

    mov edi, ebx                    ; 设置 EDI ESI
    mov esi, ebp
    add esi, KERNEL_BUF_ADDR

    test edx, 0xFFFFFE00            ; 检查是否小于 512 字节
    jz .low_512

    mov ecx, 512
    sub ecx, ebp                    ; 读取 512 - ebp 个字节
    rep movsb

    sub edx, 512                    ; edx -= 512 - ebp
    add edx, ebp

    mov ebx, edi
    xor ebp, ebp
    jmp .read_next_sector
.low_512:
    mov ecx, edx
    rep movsb
.ok:
    popad
    clc
    ret
.err:
    popad
    stc
    ret


halt:
    hlt
    jmp halt


; data
; Strings
err_E820_str        db 0dh, 0ah, 'Cannot detect memory infomation.', 0dh, 0ah
                    db 'Maybe the BIOS does not support <int 0x15, eax=0xE820>?', 0
get_mem_info_str    db 'Faild to get memory info. (mem_lower, mem_upper)'
hello_str           db 0ah, 0ah, 'Door Loader', 0ah, 'By Zhan zixuan, 2026', 0ah, 0ah, 0,
hex_upper_str       db '0123456789ABCDEF', 0
mmap_head_str       db 'No.  StartAddress---- EndAddress------ Length---------- Type---- Comment', 0ah, 0
unknown_str         db 'Unknown', 0
available_str       db 'Available', 0
reserved_str        db 'Reserved', 0
acpi_reclaim_str    db 'ACPI Reclaim', 0
acpi_nvs_str        db 'ACPI NVS', 0
defective_mem_str   db 'Defective', 0
init_floppy_err_str db 'Error when initializing floppy.', 0ah, 0
floppy_reca_err_str db 'Error when recalibrating floppy.', 0ah, 0
floppy_seek_err     db 'Error when seeking floppy head.', 0ah, 0
floppy_read_err     db 'Error when reading data from floppy.', 0ah, 0
read_fat_err_str    db 'Error when reading FAT table.', 0ah, 0
read_root_err_str   db 'Error when reading root directory.', 0ah, 0
cannot_loadfile_str db 'Cannot load file.', 0ah, 0
too_long_key_str    db 'CFG Parse Error: Key was too long.', 0ah, 0
unknown_key_err_str db 'CFG Parse Error: Unknown key: ', 0
too_long_val_str    db 'CFG Parse Error: The value of this key was too long: ', 0
invaild_key_str     db 'CFG Parse Error: Invaild key: ', 0
nofind_ent_root_str db 'Did not find entry in root directory: ', 0
nofind_ent_sub_str  db 'Did not find entry in directory: ', 0
invaild_name_str    db 'Invaild file name: ', 0
invaild_elfhead_str db 'Invaild ELF head.', 0
floppy_too_many_str db 0ah, 'Too many results from FDC. CPU halt...', 0
null_handler_str    db 0ah, 'Null intrrupt handler was fired. CPU halt...', 0
found_boot_name_str db 'Boot name: ', 0
print_path_str      db 'Kernel path: ', 0
print_cmdline_str   db 'Kernel cmdline: ', 0
mem_lower_str       db 'Lower memory (in KiB): ', 0
mem_upper_str       db 'Upper memory (in KiB): ', 0
err_check_sum_str   db 'Wrong Multiboot checksum!', 0ah, 0
invaild_kernel_str  db 'Invaild kernel image! (No Multiboot sign)', 0ah, 0
ok_kernel_str       db 'Loading kernel...', 0
cannot_loadker_str  db 'Cannot load kernel to memory.', 0
bootloader_name_str db 'Door Loader', 0

bootname_str        db 'bootname', 0
kernel_cmdline_str  db 'kernel', 0

loader_cfg_path_str db 'boot/loader.cfg', 0


; 全局变量
align 8
mmap_size           dd 0
cursor_col          dd 0
cursor_row          dd 0
floppy_flag         dd 0
floppy_read_count   dd 0
floppy_cur_track    dd -1
mboot_flags         dd -1
tick                dd 0
kernel_size         dd 0

floppy_result_start:
floppy_result0      db 0xFF
floppy_result1      db 0xFF
floppy_result2      db 0xFF
floppy_result3      db 0xFF
floppy_result4      db 0xFF
floppy_result5      db 0xFF
floppy_result6      db 0xFF
floppy_result7      db 0xFF
floppy_result8      db 0xFF
floppy_result9      db 0xFF
floppy_result10     db 0xFF
floppy_result11     db 0xFF
floppy_result_end:


; GDTR 全局描述符表寄存器
align 8
gdtr_val:
gdt_size            dw GDT_SIZE
gdt_address         dd GDT_ADDR

; IDTR 寄存器加载结构 (6 字节)
align 8
idtr_val:
idt_size            dw IDT_SIZE
idt_address         dd IDT_ADDR
