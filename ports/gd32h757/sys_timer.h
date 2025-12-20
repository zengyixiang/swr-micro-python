#ifndef __SYS_TIMER_H__
#define __SYS_TIMER_H__



#include "py/obj.h"

void timer_init0(void);

extern const mp_obj_module_t delay_module;
// extern const struct _mp_obj_type_t pyb_delay_type;

extern const struct _mp_obj_type_t pyb_timer_type;

void py_del_all_timer(void);
#endif //__SYS_TIMER_H__
