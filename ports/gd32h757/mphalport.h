#ifndef __MPHAIPORT_H__
#define __MPHAIPORT_H__

#include "py/ringbuf.h"
#include "RTOS/FreeRTOS/Source/include/FreeRTOS.h"
#include "RTOS/FreeRTOS/Source/include/task.h"
#include "RTOS/FreeRTOS/Source/include/timers.h"

extern ringbuf_t stdin_ringbuf;

void mp_hal_set_interrupt_char(int c); 
void mp_hal_wake_main_task(void);
void mp_hal_wake_main_task_from_isr(void);
uint64_t gd32_get_time();
void mp_hal_delay_us(mp_uint_t us);
void mp_hal_delay_ms(mp_uint_t ms);

static inline mp_uint_t mp_begin_atomic_section(void) {
    taskENTER_CRITICAL();
    return 0;
}

static inline void mp_end_atomic_section(mp_uint_t state) {
    (void)state;
    taskEXIT_CRITICAL();
}

#define MICROPY_BEGIN_ATOMIC_SECTION()     mp_begin_atomic_section()
#define MICROPY_END_ATOMIC_SECTION(state)  mp_end_atomic_section(state)

#endif //__MPHAIPORT_H__
