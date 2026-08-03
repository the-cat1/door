// door/main.c
// Enter point of kernel.

#include "printk.h"
#include "lib/sprintf.h"

void kmain(void)
{
    printk("%10s\n", NULL);
    printk("%10s\n", "hello");
    printk("%10s\n", "world");
    printk("%10s\n", "kernel!");
    printk("%0+10d\n", 25); // something wrong
    printk("%0 10d\n", 25);

    while (1)
        ;
}
