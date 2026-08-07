// door/main.c
// Enter point of kernel.

#include "printk.h"
#include "lib/sprintf.h"

void kmain(void)
{
    printk("%#5.1x\n", 0, 23);
    printk("PTR %p\n", NULL);
    printk("PTR %u\n", 0xB8000);
    printk("PTR %p\n", 0xc0000000);

    while (1)
        ;
}
