// kernel/include/device/timer.h

#ifndef __DEVICE_TIMER_H
#define __DEVICE_TIMER_H

// in Hz
#define TICKS_FREQ (unsigned long)(1000)

void init_timer();

#ifdef __DEVICE_TIMER_C
#define extern
#endif
extern unsigned long ticks;
#ifdef __DEVICE_TIMER_C
#undef extern
#undef __DEVICE_TIMER_C
#endif

#endif
