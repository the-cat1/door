; Door
; door/boot/bootsect.asm

; Boot sector to load the kernel from disk.
; Just for FAT12 and 1.44MB floppy disk.


LOADER_BASE                 equ 0
LOADER_ADDR                 equ 8000h

SECTOR_DIR_READ_ADDR        equ 500h
SECTOR_FAT1_READ_ADDR       equ 700h
SECTOR_FAT2_READ_ADDR       equ 900h


[org 0x7c00]
[bits 16]


BPB_Data:
    jmp short start
    nop

BS_OEMName              db      'DOOR    '		; 长度必须为 8，不足以空格填充
BPB_BytesPerSec         dw      512
BPB_SecPerClus          db      1
BPB_RsvdSecCnt          dw      1
BPB_NumFATs             db      2
BPB_RootEntCnt          dw      224             ; RootDirSectors * 512 / 32 = 224
BPB_TotSec16            dw      2880            ; 1440 * 1024 / 512 = 2880   (fp: 1.44MB)
BPB_Media               db      0xf0
BPB_FATSz16             dw      9
BPB_SecPerTrk           dw      18
BPB_NumHeads            dw      2
BPB_HiddSec             dd      0
BPB_TotSec32            dd      0
BS_DrvNum               db      0
BS_Reserved1            db      0
BS_BootSig              db      0x29
BS_VolID                dd      0
BS_VolLab               db      'Door kernel'     ; 必须 11bit，不足用空格补齐
BS_FileSysType          db      'FAT12   '        ; 必须 8bit，不足用空格补齐


start:
    ; Setup the stack
    ; All the segments are set to 0, and the stack pointer is set to 0x7c00
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov di, ax
    mov sp, 0x7c00
    sti

    ; Lookup /boot/
    mov ax, 19
    xor dx, dx
    mov si, boot_dirname
    call lookup_entry
    test ax, ax
    jz error

    ; Lookup /boot/loader.bin
    inc dx
    mov si, loader_filename
    call lookup_entry
    test ax, ax
    jz error

    ; Load /boot/loader.bin
    mov bx, LOADER_BASE * 16 + LOADER_ADDR
.load_loader:
    add ax, 31
    call load_one_sector
    sub ax, 31
    call lookup_entry_next_sector
    test ax, ax
    jz .goto_loader
    add bx, 512
    jmp .load_loader

.goto_loader:
    ; Jump to the loader.
    jmp LOADER_BASE:LOADER_ADDR


; Print string pointed to by SI
print_string:
    push ax
.print_loop:
    lodsb                   ; Load byte at DS:SI into AL and increment SI
    cmp al, 0               ; Check for null terminator
    je .done                ; If null terminator, we're done
    mov ah, 0x0e            ; BIOS teletype function
    int 0x10                ; Call BIOS video interrupt
    jmp .print_loop         ; Repeat for next character
.done:
    pop ax
    ret


; Find the directory entry for the file with name in DS:SI (DS is 0!)
; AX: Start LBA(root mode) or start cluster(sub-dir mode)
; dx: 0 if Root directory mode, else sub-directory mode.
; DS:SI: The filename to search for (11 bytes, padded with spaces)
; Returns the entry's start sector in AX if found, or 0 if not found
lookup_entry:
    push bx
    push cx
    push di
    mov [temp_var], si

    test dx, dx
    jz .load_root
.load_sub:
    add ax, 31
.load_root:
    mov bx, SECTOR_DIR_READ_ADDR
    call load_one_sector
.entry_loop:
    mov di, bx
    mov si, [temp_var]
    mov cx, 11
    cld                         ; set direction
    repe cmpsb
    je .found
    add bx, 32
    cmp bx, SECTOR_DIR_READ_ADDR + 512
    jb .entry_loop

    ; Reached the end of the sector, check if there is a next sector in root directory
    test dx, dx
    jz .root_mode
.sub_dir_mode:
    sub ax, 31
    call lookup_entry_next_sector
    jnz .load_sub
    jmp .err
.root_mode:
    inc ax
    cmp ax, 19 + 14
    jb .load_root
    jmp .err
.err:
    xor ax, ax
    jmp .return
.found:
    mov ax, [bx+26]              ; The start sector is at offset 26
.return:
    pop di
    pop cx
    pop bx
    ret


; Find the next sector in FAT
; AX: The current sector in FAT table
; Returns 0 if there is no next sector, or the next sector number in AX
lookup_entry_next_sector:
    push bx
    push cx
    push dx

    mov bx, 3
    mul bx
    mov cx, ax
    and cx, 1
    shr ax, 1
    ; NOW ax: The byte of the FAT table, cx: 0 if even, 1 odd

    mov dx, ax
    and dx, 0x01FF             ; DX: The offset in the FAT sector
    add dx, SECTOR_FAT1_READ_ADDR
    ; DX: The address of the FAT entry for the current sector
    shr ax, 9               ; AX: The sector number of the FAT table
    inc ax

    cmp ax, [fat_1]
    je .jmp_fat
    mov [fat_1], ax

    ; load 2 fat sectors
    mov bx, SECTOR_FAT1_READ_ADDR
    call load_one_sector
    inc ax
    mov bx, SECTOR_FAT2_READ_ADDR
    call load_one_sector

.jmp_fat:
    mov bx, dx
    jcxz .FAT_even

.FAT_odd:
    mov al, [es:bx]
    mov ah, [es:bx+1]
    shr ax, 4
    jmp .cmpare

.FAT_even:
    mov al, [es:bx]
    mov ah, [es:bx+1]
    and ax, 0x0fff

.cmpare:
    cmp ax, 0xff0               ; ax >= 0xff0 means eof, unused, or bad cluster
    jbe .return
    xor ax, ax                  ; No next sector
.return:
    pop dx
    pop cx
    pop bx
    ret


; Load one sector from disk into memory at ES:BX
; ax: LBA of the sector to load
; es:bx: destination address in memory
load_one_sector:
    push ax
    push cx
    push dx

    push bx
    mov bl, 18              ; Sectors per track
    div bl                  ; ax/bl -> 商在 al, 余数在 ah
    inc ah                  ; 扇区号从1开始，所以加1
    mov cl, ah              ; CL = sector number
    mov ch, al
    shr ch, 1               ; CH = cylinder number (divide by 2)
    mov dh, al
    and dh, 1               ; DH = head number (0 or 1)
    pop bx

    mov dl, 0x00            ; Drive number (0x00 for floppy)

.trying:
    mov ah, 0x02            ; BIOS function: Read sectors
    mov al, 1               ; Number of sectors to read
    int 0x13                ; Call BIOS disk interrupt
    jc .trying              ; If carry flag is set, retry

    pop dx
    pop cx
    pop ax
    ret


error:
    mov si, error_str
    call print_string
stop:
    hlt
    jmp stop


; Data
; Strings
error_str       db 'Failed loading the loader!', 0dh, 0ah, 0

boot_dirname    db 'BOOT       '
loader_filename db 'LOADER  BIN'

; Some vars
temp_var        dw 0
fat_1           dw 0


; Fill the rest of the boot sector with zeros and add the boot sector signature
times 510 - ($ - $$) db 0   ; Fill the rest of the boot sector with zeros
dw 0xAA55                   ; Boot sector signature
