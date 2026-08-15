/* linker.ld */


V_OFFSET = 0xC0000000;
KERNEL_PMA = 0x00100000;
KERNEL_VMA = KERNEL_PMA + V_OFFSET;

/* 内核入口点 */
ENTRY(start)

SECTIONS {
    . = KERNEL_PMA;

    .boot BLOCK(4K) : AT(KERNEL_PMA) ALIGN(4K) {
        *(.multiboot)
        *(.boot.text)
        *(.boot.data)
    }

    . = KERNEL_VMA + SIZEOF(.boot);

    /* 让 .text 段及其后的 PADDR 为 VADDR - V_OFFSET */
    .text BLOCK(4K) : AT(ADDR(.text) - V_OFFSET) ALIGN(4K) {
        __text_start = .;
        *(.text)
        __text_end = .;
    }

    /* 只读数据 */
    .rodata BLOCK(4K) : AT(ADDR(.rodata) - V_OFFSET) ALIGN(4K) {
        __rodata_start = .;
        *(.rodata)
        __rodata_end = .;
    }

    /* 已初始化数据 */
    .data BLOCK(4K) : AT(ADDR(.data) - V_OFFSET) ALIGN(4K) {
        __data_start = .;
        *(.data)
        __data_end = .;
    }

    /* 未初始化数据 (BSS) */
    .bss BLOCK(4K) : AT(ADDR(.bss) - V_OFFSET) ALIGN(4K) {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    }

    __kernel_end = .;
}
