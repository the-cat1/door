// kernel/main.c
// Enter point of kernel.

#include "boot.h"
#include "video.h"

void kmain(void)
{
    init_video();
    copy_multiboot_info();

    while (1)
        ;
}
