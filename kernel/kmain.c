// door/main.c
// Enter point of kernel.

#include "printk.h"
#include "lib/sprintf.h"

void kmain(void)
{
    printk("Enter kmain().");

    while (1)
        ;
}
