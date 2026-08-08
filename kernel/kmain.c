// door/main.c
// Enter point of kernel.

#include "printk.h"
#include "boot.h"

void kmain(void)
{
    if (copy_multiboot_info())
    {
        printk("NOOOOOO");
    }

    while (1)
        ;
}
