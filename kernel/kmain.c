// door/main.c
// Enter point of kernel.

#include <stddef.h>
#include <stdint.h>

#include "printk.h"

void kmain(void)
{
    intmax_t n = -1;
    printk("~~~%10.0u%jn~~~\n", 20, &n);
    printk("~~~%-10lc~~~\n", '?');
    printk("~~~%-10ji~~~\n", n);
    
    while (1)
        ;
}
