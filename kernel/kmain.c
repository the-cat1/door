// door/main.c
// Enter point of kernel.

#include "assert.h"
#include "boot.h"

void kmain(void)
{
    copy_multiboot_info();
    assert(1==0);
    while (1)
        ;
}
