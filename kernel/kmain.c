// door/main.c
// Enter point of kernel.

#include "printk.h"
#include "lib/sprintf.h"

void kmain(void)
{
    printk("%#-5X%#5o", 31, 23);

    while (1)
        ;
}
