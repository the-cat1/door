// kernel/main.c
// Enter point of kernel.

#include <stdbool.h>

#include "asm.h"
#include "boot.h"
#include "irq.h"
#include "video.h"

void kmain(void)
{
    init_video();
    init_irq();
    copy_multiboot_info();

    while (true)
        hlt();
}
