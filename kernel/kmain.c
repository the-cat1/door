// door/main.c
// Enter point of kernel.

#include "printk.h"
#include "boot.h"

void kmain(void)
{
    copy_multiboot_info();

    while (1)
        ;
}
