// kernel/include/printk.h

#ifndef __PRINTK_H
#define __PRINTK_H

#define VIDEO_MEMORY ((char *)0xB8000)
#define ATTRIBUTE 0x07
#define COLS 80
#define ROWS 25

void printk(const char *format, ...);

#endif /* __PRINTK_H */
