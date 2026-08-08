// kernel/include/boot.h

#ifndef __BOOT_H
#define __BOOT_H

#include <stdint.h>

/* boot/multiboot.c */

#define MMAP_L_MAX_COUNT 4      // 最大低端可用 mmap 数量，多的会被忽略
#define MMAP_U_MAX_COUNT 16     // 最大高端可用 mmap 数量，多的会被忽略
#define BOOT_LOADER_NAME_LEN 64 // boot_loader_name 长度

// See https://www.gnu.org/software/grub/manual/multiboot/html_node/Boot-information-format.html#Boot-information-format

// Multiboot info flag bits
#define MB_INFO_MEM (1 << 0)
#define MB_INFO_BOOTDEV (1 << 1)
#define MB_INFO_CMDLINE (1 << 2)
#define MB_INFO_MODS (1 << 3)
#define MB_INFO_AOUT_SYMS (1 << 4)
#define MB_INFO_ELF_SHDR (1 << 5)
#define MB_INFO_MMAP (1 << 6)
#define MB_INFO_DRIVES (1 << 7)
#define MB_INFO_CONFIG_TABLE (1 << 8)
#define MB_INFO_BOOT_LOADER_NAME (1 << 9)
#define MB_INFO_APM_TABLE (1 << 10)
#define MB_INFO_VBE (1 << 11)
#define MB_INFO_FRAMEBUFFER (1 << 12)

// Multiboot mmap types
#define MB_MMAP_AVAILABLE 1
#define MB_MMAP_ACPI 3
#define MB_MMAP_DEFECTIVE 5

// Multiboot information, provides by loader
struct multiboot_info
{
    // offset 0: flags
    uint32_t flags;       // 0
    uint32_t mem_lower;   // 4
    uint32_t mem_upper;   // 8
    uint32_t boot_device; // 12
    uint32_t cmdline;     // 16
    uint32_t mods_count;  // 20
    uint32_t mods_addr;   // 24
    union
    {
        struct
        {
            uint32_t tabsize;  // 28
            uint32_t strsize;  // 32
            uint32_t addr;     // 36
            uint32_t reserved; // 40
        } aout;
        struct
        {
            uint32_t num;   // 28
            uint32_t size;  // 32
            uint32_t addr;  // 36
            uint32_t shndx; // 40
        } elf;
    } syms;                      // 28
    uint32_t mmap_length;        // 44
    uint32_t mmap_addr;          // 48
    uint32_t drives_length;      // 52
    uint32_t drives_addr;        // 56
    uint32_t config_table;       // 60
    uint32_t boot_loader_name;   // 64
    uint32_t apm_table;          // 68
    uint32_t vbe_control_info;   // 72
    uint32_t vbe_mode_info;      // 76
    uint16_t vbe_mode;           // 80
    uint16_t vbe_interface_seg;  // 82
    uint16_t vbe_interface_off;  // 84
    uint16_t vbe_interface_len;  // 86
    uint64_t framebuffer_addr;   // 88
    uint32_t framebuffer_pitch;  // 96
    uint32_t framebuffer_width;  // 100
    uint32_t framebuffer_height; // 104
    uint8_t framebuffer_bpp;     // 108
    uint8_t framebuffer_type;    // 109
    union
    {
        struct
        {
            uint32_t framebuffer_palette_addr; // 110
            uint16_t framebuffer_palette_num_colors;
        };
        struct
        {
            uint8_t framebuffer_red_field_position;   // 110
            uint8_t framebuffer_red_mask_size;        // 111
            uint8_t framebuffer_green_field_position; // 112
            uint8_t framebuffer_green_mask_size;      // 113
            uint8_t framebuffer_blue_field_position;  // 114
            uint8_t framebuffer_blue_mask_size;       // 115
        };
    } color_info; // 110
} __attribute__((packed));

// multiboot module
struct multiboot_mod
{
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t string;
    uint32_t reserved;
} __attribute__((packed));

// multiboot mmap
struct multiboot_mmap
{
    uint32_t size;   // -4
    uint64_t base;   // 0
    uint64_t length; // 8
    uint32_t type;   // 16
} __attribute__((packed));

#ifdef __BOOT_MULTIBOOT_C
#define extern
#endif
extern struct multiboot_mmap mmaps_l[MMAP_L_MAX_COUNT];
extern int mmaps_l_count;
extern struct multiboot_mmap mmaps_u[MMAP_U_MAX_COUNT];
extern int mmaps_u_count;
extern char boot_loader_name[BOOT_LOADER_NAME_LEN];
#ifdef __BOOT_MULTIBOOT_C
#undef extern
#undef __BOOT_MULTIBOOT_C
#endif

int copy_multiboot_info();

/* boot/boot.asm */
#define GDT_CODE_SEG (1 << 3)
#define GDT_DATA_SEG (2 << 3)

extern struct multiboot_info *mboot_info;

/* linker.ld */
extern void *V_OFFSET;
extern void *__bss_start;
extern void *__bss_end;

#endif
