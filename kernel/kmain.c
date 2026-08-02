// door/main.c
// Enter point of kernel.

#include <stddef.h>
#include <stdint.h>

#include "printk.h"

void kmain(void)
{
    printk("~~~%10.0u~~~\n", 0);
    printk("~~~%-10.0u~~~\n", 20);

    while (1)
        ;
}
