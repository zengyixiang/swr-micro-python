#ifndef __MPHAIPORT_H__
#define __MPHAIPORT_H__

#include "py/ringbuf.h"

extern ringbuf_t stdin_ringbuf;

void mp_hal_set_interrupt_char(int c); 
void mp_hal_wake_main_task(void);
void mp_hal_wake_main_task_from_isr(void);
uint64_t gd32_get_time();
void mp_hal_delay_us(mp_uint_t us);
void mp_hal_delay_ms(mp_uint_t ms);
#endif //__MPHAIPORT_H__
