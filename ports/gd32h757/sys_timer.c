#include "sys_timer.h"
#include "py/runtime.h"
#include "py/gc.h"
#include <stdio.h>
#include <string.h>
// #include "debug/debug.h"
#include "py/objmodule.h"
// #include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define SYS_TIMER_NUM       10

#define PYB_TIMER_OBJ_ALL_NUM MP_ARRAY_SIZE(MP_STATE_PORT(pyb_timer_obj_all))



#ifdef  GD32H7XX
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#else

#endif


static mp_obj_t delay_ms(mp_obj_t time_ms) {
	if(get_start_flag() == TRUE) 
		return mp_const_none;
    mp_int_t delay_ms = mp_obj_get_int(time_ms);
    if (MP_STATE_VM(sched_state) == MP_SCHED_IDLE)
    {
        MP_THREAD_GIL_EXIT();
        vTaskDelay(delay_ms);
        MP_THREAD_GIL_ENTER();
    }
    else
    {
        vTaskDelay(delay_ms);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(ms_obj, delay_ms);

static mp_obj_t delay_s(mp_obj_t time_s) {
	if(get_start_flag() == TRUE) 
		return mp_const_none;
    mp_int_t delay_s = mp_obj_get_int(time_s);
    if (MP_STATE_VM(sched_state) == MP_SCHED_IDLE)
    {
        MP_THREAD_GIL_EXIT();
        vTaskDelay(delay_s * 1000);
        MP_THREAD_GIL_ENTER();
    }
    else
    {
        vTaskDelay(delay_s * 1000);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(s_obj, delay_s);

static const mp_rom_map_elem_t delay_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_ms), MP_ROM_PTR(&ms_obj) },
    { MP_ROM_QSTR(MP_QSTR_s), MP_ROM_PTR(&s_obj) },
};

static MP_DEFINE_CONST_DICT(delay_locals_dict, delay_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    pyb_delay_type,
    MP_QSTR_delay,
    MP_TYPE_FLAG_NONE,
    locals_dict, &delay_locals_dict
    );

//-----------------------------------------------------------------------------------------------

typedef struct _pyb_timer_obj_t {
    mp_obj_base_t base;
    uint8_t tim_id;
    mp_obj_t callback;
    TimerHandle_t  timerhandle;
} pyb_timer_obj_t;



void py_del_all_timer(void)
{
    for (uint i = 0; i < PYB_TIMER_OBJ_ALL_NUM; i++) {
        if(MP_STATE_PORT(pyb_timer_obj_all)[i] != NULL)
        {
            if(MP_STATE_PORT(pyb_timer_obj_all)[i]->timerhandle != NULL)
            {
                bool res = xTimerDelete(MP_STATE_PORT(pyb_timer_obj_all)[i]->timerhandle,100);
                if(res != pdPASS)
                {
//                    debug_printf("del_all_timer error %d \r\n",i);
                }
                else
                {
//                   debug_printf("del_all_timer ok %d \r\n",i);
                }
            }
            m_del_obj(pyb_timer_obj_t,MP_STATE_PORT(pyb_timer_obj_all)[i]);
            MP_STATE_PORT(pyb_timer_obj_all)[i] = NULL;
        }
    }
}
extern void interrupt_to_thread(void* callback,void* para);
static void sys_timer_do(TimerHandle_t xTimer)
{
    int* timer_id = pvTimerGetTimerID(xTimer);
    pyb_timer_obj_t *tim = MP_STATE_PORT(pyb_timer_obj_all)[*timer_id - 1];
    if (tim == NULL) {
        return;
    }
    mp_obj_t callback = tim->callback;
    #if 1
    if (callback != mp_const_none) {
        interrupt_to_thread(callback,MP_OBJ_FROM_PTR(tim));
    }
    #else
    if (callback != mp_const_none) {
        mp_sched_lock();
//        gc_lock();
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            mp_call_function_1(callback, MP_OBJ_FROM_PTR(tim));
            nlr_pop();
        } else {
//            tim->callback = mp_const_none;
            mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
        }
//        gc_unlock();
        mp_sched_unlock();
    }
    #endif
}



void timer_init0(void) {
    for (uint i = 0; i < PYB_TIMER_OBJ_ALL_NUM; i++) {
        MP_STATE_PORT(pyb_timer_obj_all)[i] = NULL;
    }
}


static mp_obj_t _timer_init(size_t n_args, const mp_obj_t *args) {
    pyb_timer_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t TimerPeriodInTicks = mp_obj_get_int(args[1]);
    bool timer_is_xAutoReload = mp_obj_is_true(args[2]);
    mp_obj_t callback = args[3];
    if (callback == mp_const_none) {
        mp_raise_ValueError(MP_ERROR_TEXT("callback must be None or a callable object"));
    }
    else if(mp_obj_is_callable(callback))
    {
        self->callback = callback;
        char name_id[2];//(char)(self->tim_id + '0');
        name_id[0] = (char)(self->tim_id + '0');
        name_id[1] = '\0';
        self->timerhandle = xTimerCreate(name_id,TimerPeriodInTicks,timer_is_xAutoReload,(void*)&self->tim_id,sys_timer_do);
    }
    else
    {
        mp_raise_ValueError(MP_ERROR_TEXT("callback must be None or a callable object"));
    }
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(timer_init_obj,4,_timer_init);


static mp_obj_t timer_start(mp_obj_t self_in) {
    pyb_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if(self->timerhandle == NULL) return mp_const_none;
    bool res = xTimerStart(self->timerhandle,100);
    if(res != pdPASS)
    {
        mp_raise_ValueError(MP_ERROR_TEXT("timer_start error"));
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(timer_start_obj,timer_start);

static mp_obj_t timer_stop(mp_obj_t self_in) {
    pyb_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if(self->timerhandle == NULL) return mp_const_none;
    bool res = xTimerStop(self->timerhandle,100);
    if(res != pdPASS)
    {
        mp_raise_ValueError(MP_ERROR_TEXT("timer_stop error"));
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(timer_stop_obj,timer_stop);

static mp_obj_t timer_change_period(mp_obj_t self_in,mp_obj_t period) {
    pyb_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t change_period = mp_obj_get_int(period);
    if(self->timerhandle == NULL) return mp_const_none;
    bool res = xTimerChangePeriod(self->timerhandle,change_period,100);
    if(res != pdPASS)
    {
        mp_raise_ValueError(MP_ERROR_TEXT("timer_change_period error"));
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(timer_change_period_obj,timer_change_period);

static mp_obj_t timer_get_period(mp_obj_t self_in) {
    pyb_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if(self->timerhandle == NULL) 
    {
        return mp_obj_new_int(0);
    }
    mp_int_t period = xTimerGetPeriod(self->timerhandle);
    return mp_obj_new_int(period);
}
static MP_DEFINE_CONST_FUN_OBJ_1(timer_get_period_obj,timer_get_period);

static mp_obj_t timer_is_active(mp_obj_t self_in) {
    pyb_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if(self->timerhandle == NULL) 
    {
        return mp_obj_new_bool(pdFALSE);
    }
    mp_int_t res = xTimerIsTimerActive(self->timerhandle);
    return mp_obj_new_bool(res);
}
static MP_DEFINE_CONST_FUN_OBJ_1(timer_is_active_obj,timer_is_active);

static mp_obj_t timer_delete(mp_obj_t self_in) {
    pyb_timer_obj_t *self = MP_OBJ_TO_PTR(self_in);
    MP_STATE_PORT(pyb_timer_obj_all)[self->tim_id - 1] = NULL;
    if(self->timerhandle != NULL) 
    {
        bool res = xTimerDelete(self->timerhandle,100);
        if(res != pdPASS)
        {
            mp_raise_ValueError(MP_ERROR_TEXT("timer_delete error"));
        }
        self->timerhandle = NULL;
    }
    m_del_obj(pyb_timer_obj_t,self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(timer_delete_obj,timer_delete);

static mp_obj_t pyb_timer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    mp_int_t tim_id = mp_obj_get_int(args[0]);
    if (tim_id <= 0 || tim_id > SYS_TIMER_NUM || MP_STATE_PORT(pyb_timer_obj_all)[tim_id - 1] != NULL) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Timer(%d) doesn't exist"), tim_id);
    }

    pyb_timer_obj_t * tim = m_new_obj(pyb_timer_obj_t);
    memset(tim, 0, sizeof(*tim));
    tim->base.type = &pyb_timer_type;
    tim->tim_id = tim_id;
    tim->callback = mp_const_none;
    tim->timerhandle = NULL;
    MP_STATE_PORT(pyb_timer_obj_all)[tim_id - 1] = tim;


    return MP_OBJ_FROM_PTR(tim);
}


static const mp_rom_map_elem_t pyb_timer_locals_dict_table[] = {
    //定时器初始化
    //参数1：定时器周期  ms
    //参数2：是否重复执行  1：是  0：否
    //参数3：回调函数
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&timer_init_obj) },
    //使能定时器
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&timer_start_obj) },
    //停止定时器
    { MP_ROM_QSTR(MP_QSTR_stop), MP_ROM_PTR(&timer_stop_obj) },
    //切换定时周期：
    //参数1：新的定时周期  ms
    { MP_ROM_QSTR(MP_QSTR_change_period), MP_ROM_PTR(&timer_change_period_obj) },
    //获取定时周期
    { MP_ROM_QSTR(MP_QSTR_get_period), MP_ROM_PTR(&timer_get_period_obj) },
    //定时器是否激活
    { MP_ROM_QSTR(MP_QSTR_is_active), MP_ROM_PTR(&timer_is_active_obj) },
    //删除定时器
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&timer_delete_obj) },
};

static MP_DEFINE_CONST_DICT(pyb_timer_locals_dict, pyb_timer_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    pyb_timer_type,
    MP_QSTR_timer,
    MP_TYPE_FLAG_NONE,
    make_new, pyb_timer_make_new,
    locals_dict, &pyb_timer_locals_dict
    );

MP_REGISTER_ROOT_POINTER(struct _pyb_timer_obj_t *pyb_timer_obj_all[SYS_TIMER_NUM]);

