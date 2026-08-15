// SPDX-FileCopyrightText: 2026 Zhan Zixuan
//
// SPDX-License-Identifier: MIT

#include "lib/string.h"
#include "assert.h"
#include "printk.h"
#include "panic.h"
#define __BOOT_MULTIBOOT_C
#include "boot.h"

static const char *get_mmap_type_string(uint32_t type)
{
    switch (type) {
    case MB_MMAP_AVAILABLE:
        return "available";

    case MB_MMAP_ACPI:
        return "acpi information";

    case MB_MMAP_DEFECTIVE:
        return "defective";

    default:
        return "reserved";
    }
}

static void print_mmap_entry(struct multiboot_mmap *entry, int i, const char *pos)
{
    printk(
        "%-3s %-3d %.16jx %.16jx %u(%s)", pos, i, entry->base, entry->length, entry->type,
        get_mmap_type_string(entry->type)
    );
}

static void print_mmap()
{
    if (mmaps_l_count <= 0 && mmaps_u_count <= 0)
        return;

    printk("Pos Id  Base Address     Length           Type");
    for (int i = 0; i < mmaps_l_count; i++) {
        struct multiboot_mmap entry = mmaps_l[i];
        print_mmap_entry(&entry, i, "LOW");
    }
    for (int i = 0; i < mmaps_u_count; i++) {
        struct multiboot_mmap entry = mmaps_u[i];
        print_mmap_entry(&entry, i, "HIG");
    }
}

static int copy_mmap()
{
    struct multiboot_mmap *entries = (void *)mboot_info->mmap_addr;

    if (mboot_info->mmap_length < 20) {
        printk("mmap_length < 20");
        return -1;
    }

    if (entries[0].size != 20) {
        printk("Unsupport mmap size: %d", entries[0].size);
        return -1;
    }

    uint32_t count = mboot_info->mmap_length / sizeof(struct multiboot_mmap);
    for (uint32_t i = 0; i < count; i++) {
        struct multiboot_mmap entry = entries[i];

        if (entry.type != MB_MMAP_AVAILABLE) // only copy available mmap entries
            continue;

        if (entry.base < 0x100000) {
            if (mmaps_l_count >= MMAP_L_MAX_COUNT)
                continue;

            memcpy(&mmaps_l[mmaps_l_count], &entry, sizeof(entry));
            mmaps_l_count++;
        } else {
            if (mmaps_u_count >= MMAP_U_MAX_COUNT)
                continue;

            memcpy(&mmaps_u[mmaps_u_count], &entry, sizeof(entry));
            mmaps_u_count++;
        }
    }

    if (mmaps_l_count <= 0 || mmaps_u_count <= 0)
        return -1;

    print_mmap();

    return 0;
}

static int make_mmap()
{
    if (!(mboot_info->flags & MB_INFO_MEM))
        return -1;

    assert(MMAP_L_MAX_COUNT >= 1 && MMAP_U_MAX_COUNT >= 1);

    if (!(mboot_info->flags & MB_INFO_MEM)) {
        printk("No mem_*.");
        return -1;
    }

    mmaps_l[0] = (struct multiboot_mmap){
        .base = 0,
        .length = mboot_info->mem_lower * 1024, // in kilobytes
        .type = MB_MMAP_AVAILABLE
    };
    mmaps_l_count = 1;

    mmaps_u[0] = (struct multiboot_mmap){
        .base = 0x100000,
        .length = mboot_info->mem_upper * 1024, // in kilobytes
        .type = MB_MMAP_AVAILABLE
    };
    mmaps_u_count = 1;

    print_mmap();

    return 0;
}

/**
 * @brief 复制必要的 Multiboot 信息
 */
void copy_multiboot_info()
{
    if (mboot_info->flags & MB_INFO_MEM) {
        printk("mem_upper: %u", mboot_info->mem_upper);
        printk("mem_lower: %u", mboot_info->mem_lower);
    }

    if (mboot_info->flags & MB_INFO_BOOTDEV)
        printk("boot_devics: %#.8x", mboot_info->boot_device);

    if (mboot_info->flags & MB_INFO_CMDLINE)
        printk("cmdline: %s", mboot_info->cmdline);

    if (mboot_info->flags & MB_INFO_MODS) {
        struct multiboot_mod *mods = (struct multiboot_mod *)mboot_info->mods_addr;
        uint32_t count = mboot_info->mods_count;

        for (uint32_t i = 0; i < count; i++)
            printk("module %u(%s), %p~%p", i, mods[i].string, mods[i].mod_start, mods[i].mod_end);
    }

    if (!(mboot_info->flags & MB_INFO_MMAP && !copy_mmap())) {
        printk("cannot read mmap from multiboot_info. Try to make by mem_*.");
        if (make_mmap())
            panic("cannot make mmap");
    }

    if (mboot_info->flags & MB_INFO_BOOT_LOADER_NAME) {
        printk("boot_loader_name: %s", (char *)mboot_info->boot_loader_name);
        strncpy(boot_loader_name, (char *)mboot_info->boot_loader_name, BOOT_LOADER_NAME_LEN - 1);
        boot_loader_name[BOOT_LOADER_NAME_LEN - 1] = 0; // 保证 0 结尾
    }
}
