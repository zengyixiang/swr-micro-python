#include "sys_sensors.h"
#include "py/runtime.h"
#include "py/gc.h"
#include <string.h>
#include <stdio.h>
// #include "sensors/sensors.h"
// #include "sensors/sensors_protocol.h"


#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "py/objmodule.h"
#include "sys.h"
#include "sensors/sensors.h"

#define PYB_SENSORS_OBJ_ALL_NUM MP_ARRAY_SIZE(MP_STATE_PORT(pyb_sensors_obj_all))

typedef struct _pyb_sensors_obj_t {
    mp_obj_base_t   base;
    uint16_t        sensor_obj_number;
    Sensors_name_TypeDef sensor_name;
    uint16_t            sensor_number;
    uint16_t    sensor_channel_num;
    int16_t    sensor_ptr;
    bool        event_is_enable[SYS_SENSORS_MAX_CHANNEL_NUM][SYS_SENSORS_MAX_EVENT_NUM];
    mp_obj_t    event_condition[SYS_SENSORS_MAX_CHANNEL_NUM][SYS_SENSORS_MAX_EVENT_NUM];
    mp_obj_t    event_callback[SYS_SENSORS_MAX_CHANNEL_NUM][SYS_SENSORS_MAX_EVENT_NUM];
    mp_obj_t    user_data[SYS_SENSORS_MAX_CHANNEL_NUM][SYS_SENSORS_MAX_EVENT_NUM];
} pyb_sensors_obj_t;


void py_del_all_sensors(void)
{
    for (uint i = 0; i < PYB_SENSORS_OBJ_ALL_NUM; i++) {
        if(MP_STATE_PORT(pyb_sensors_obj_all)[i] != NULL)
        {
            m_del_obj(pyb_sensors_obj_t,MP_STATE_PORT(pyb_sensors_obj_all)[i]);
            MP_STATE_PORT(pyb_sensors_obj_all)[i] = NULL;
        }
    }
}


_Bool check_event_satisfy_condition(void * obj,uint8_t channel,uint8_t event_number)
{
    pyb_sensors_obj_t *sensor = obj;
    if (sensor == NULL) {
        return FALSE;
    }
	if(sensor->event_condition[channel][event_number] == NULL)
		return FALSE;
    mp_obj_t callback = sensor->event_condition[channel][event_number];
    mp_obj_t ret = 0;
    if (callback != mp_const_none) {
        mp_sched_lock();
//        gc_lock();
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            ret = mp_call_function_1(callback, MP_OBJ_FROM_PTR(sensor));
            nlr_pop();
        } else {
//            tim->callback = mp_const_none;
            mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
//            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Callback error"));
        }
//        gc_unlock();
        mp_sched_unlock();
    }
    if(mp_obj_get_int(ret) == 0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
extern void interrupt_to_thread(void* callback,void* para);
void run_event_callback(void * obj,uint8_t channel,uint8_t event_number)
{
    pyb_sensors_obj_t *sensor = obj;
    if (sensor == NULL) {
        return;
    }
    mp_obj_t callback = sensor->event_callback[channel][event_number];
    mp_obj_t user_data = sensor->user_data[channel][event_number];
    #if 1
    if (callback != mp_const_none) {
        // interrupt_to_thread(callback,MP_OBJ_FROM_PTR(sensor));
        interrupt_to_thread(callback,user_data);
    }
    #else
    if (callback != mp_const_none) {
        mp_sched_lock();
//        gc_lock();
        nlr_buf_t nlr;
        if (nlr_push(&nlr) == 0) {
            mp_call_function_1(callback, MP_OBJ_FROM_PTR(sensor));
            nlr_pop();
        } else {
//            tim->callback = mp_const_none;
            mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(nlr.ret_val));
//            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Callback error"));
        }
//        gc_unlock();
        mp_sched_unlock();
    }
    #endif
}

static int find_free_obj(void)
{
    for(uint16_t i = 0;i < PYB_SENSORS_OBJ_ALL_NUM;i++)
    {
        if(MP_STATE_PORT(pyb_sensors_obj_all)[i] == NULL)
        {
            return i;
        }
    }
    return -1;
}


void sys_sensors_init(void)
{
    for (uint i = 0; i < PYB_SENSORS_OBJ_ALL_NUM; i++) {
        MP_STATE_PORT(pyb_sensors_obj_all)[i] = NULL;
    }
}


static mp_obj_t sensors_info(mp_obj_t self_in,mp_obj_t type) {
    pyb_sensors_obj_t *self = MP_OBJ_TO_PTR(self_in);
    (void)self;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(sensors_info_obj,sensors_info);


static mp_obj_t sensors_get_value(mp_obj_t self_in,mp_obj_t channel) {
    pyb_sensors_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t c_value[SYS_SENSORS_MAX_CUSTOM_VALUE_NUM];
    mp_int_t i_value;
    float f_value;
    Sensors_data_type_TypeDef type = get_sensors_type(self->sensor_ptr,mp_obj_get_int(channel) - 0);
    uint8_t c_value_num = get_external_sensor_value(self->sensor_ptr,mp_obj_get_int(channel) - 0,&i_value,&f_value,c_value);
    if(type == D_TYPE_FLOAT)
    {
//        printf("get_value %d:%f\r\n",mp_obj_get_int(channel),f_value);
        return mp_obj_new_float(f_value);
    }
    else if(type == D_TYPE_INT)
    {
        return mp_obj_new_int(i_value);
    }
    else
    {
        mp_obj_t list = mp_obj_new_list(c_value_num,NULL);
        for(uint8_t i = 0;i < c_value_num;i++)
        {
            mp_obj_list_store(list,mp_obj_new_int(i),mp_obj_new_int(c_value[i]));
        }
        return list;
    }
}
static MP_DEFINE_CONST_FUN_OBJ_2(sensors_get_value_obj,sensors_get_value);


static mp_obj_t sensors_set_value(mp_obj_t self_in,mp_obj_t parameter) {
//    return mp_const_none;
    size_t t_len,a_len;
    mp_obj_t *t_items,*a_items;
    uint8_t channel,function,action;
    mp_uint_t   i_value;
    uint8_t j,send_len;
    uint8_t send_data[64];
    u8_to_float_TypeDef f_out;
    pyb_sensors_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_obj_list_get(parameter,&t_len,&t_items);
    for(uint8_t i = 0;i < t_len;)
    {
        send_len = 0;
        channel = mp_obj_get_int(t_items[i++]);
        mp_obj_list_get(t_items[i++],&a_len,&a_items);
        j = 0;
        function = mp_obj_get_int(a_items[j++]);
        action = mp_obj_get_int(a_items[j++]);
        for(;j < a_len;)
        {
            if(mp_obj_is_int(a_items[j]))
            {
                i_value = mp_obj_int_get_uint_checked(a_items[j++]);
                send_data[send_len++] = i_value >> 0;
                send_data[send_len++] = i_value >> 8;
                send_data[send_len++] = i_value >> 16;
                send_data[send_len++] = i_value >> 24;
            }
            else if(mp_obj_is_float(a_items[j]))
            {
                f_out.out = mp_obj_get_float(a_items[j++]);
                send_data[send_len++] = f_out.data[0];
                send_data[send_len++] = f_out.data[1];
                send_data[send_len++] = f_out.data[2];
                send_data[send_len++] = f_out.data[3];
            }
            else
            {
                mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("sensors_set_value args error"));
            }
        }        
        set_external_sensor_value(self->sensor_ptr,channel - 0,function,action,send_data,send_len);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(sensors_set_value_obj,sensors_set_value);


static mp_obj_t sensors_set_callback(size_t n_args, const mp_obj_t *args) {
    pyb_sensors_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    mp_int_t channel = mp_obj_get_int(args[1]);
    mp_obj_t condition = args[2];
    mp_obj_t callback = args[3];
    mp_obj_t user_data = args[4];
    mp_int_t advanced_event_number = 0;
	uint8_t condition_int_flag = 0;
    uint8_t event_number;
    if(channel <= 0 || channel > SYS_SENSORS_MAX_CHANNEL_NUM) mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("sensors_set_callback doesn't exist %d"),channel);
    condition_int_flag = mp_obj_is_int(condition);
	printf("condition_int_flag:%d\r\n",condition_int_flag);
    if(condition_int_flag)
    {
        advanced_event_number = mp_obj_get_int(condition);
        printf("advanced_event_number:%d\r\n",advanced_event_number);
        
    }
    else if (callback == mp_const_none) 
    {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("sensors_set_callback doesn't exist2 %d"),channel);
    }
    for(event_number = 0;event_number < SYS_SENSORS_MAX_EVENT_NUM;event_number++)
    {
        if(self->event_is_enable[channel - 1][event_number] != true)
            break;
    }
    if(event_number >= SYS_SENSORS_MAX_EVENT_NUM)
    {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("event_number >= SYS_SENSORS_MAX_EVENT_NUM"));
    }
//    printf("sensors_set_callback:%d %d\r\n",advanced_event_number,event_number);
    self->event_is_enable[channel - 1][event_number] = true;
    self->event_callback[channel - 1][event_number] = callback;
	if(condition_int_flag)
		self->event_condition[channel - 1][event_number] = NULL;
	else
		self->event_condition[channel - 1][event_number] = condition;
	self->user_data[channel - 1][event_number] = user_data;
    enable_external_sensor_event(self->sensor_ptr,channel - 0,event_number,advanced_event_number);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(sensors_set_callback_obj,4,sensors_set_callback);


static mp_obj_t sensors_del_callback(mp_obj_t self_in,mp_obj_t channel_in,mp_obj_t callback) {
    pyb_sensors_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t channel = mp_obj_get_int(channel_in);
    if(channel <= 0 || channel > SYS_SENSORS_MAX_CHANNEL_NUM) mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("sensors_del_callback doesn't exist %d"),channel);
    uint8_t event_number;
    for(event_number = 0;event_number < SYS_SENSORS_MAX_EVENT_NUM;event_number++)
    {
        if(self->event_callback[channel - 1][event_number] == callback)
            break;
    }
    
    disable_external_sensor_event(self->sensor_ptr,channel - 0,event_number);
//    printf("sensors_del_callback:%d %d\r\n",channel,event_number);
    self->event_is_enable[channel - 0][event_number] = false;
    self->event_condition[channel - 1][event_number] = mp_const_none;
    self->event_callback[channel - 1][event_number] = mp_const_none;
    self->user_data[channel - 1][event_number] = mp_const_none;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(sensors_del_callback_obj,sensors_del_callback);

static mp_obj_t pyb_sensors_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 2, 2, false);
    mp_int_t sensor_name = mp_obj_get_int(args[0]);
    mp_int_t sensor_number = mp_obj_get_int(args[1]);

    int free_obj= find_free_obj();
    if(free_obj == -1 || sensor_name <= 0 || sensor_number <= 0)
    {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("sensros doesn't exist %d %d %d"),free_obj,sensor_name,sensor_number);
    }
    for(uint16_t i = 0;i < SYS_SENSORS_NUM;i++)
    {
        if(MP_STATE_PORT(pyb_sensors_obj_all)[i] == NULL) continue;
        if(sensor_name == MP_STATE_PORT(pyb_sensors_obj_all)[i]->sensor_name && sensor_number == MP_STATE_PORT(pyb_sensors_obj_all)[i]->sensor_number)
        {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("sensros doesn't exist repeat %d %d %d"),free_obj,sensor_name,sensor_number);
        }
    }

    pyb_sensors_obj_t * sensor = m_new_obj(pyb_sensors_obj_t);
    memset(sensor, 0, sizeof(*sensor));
    sensor->base.type = &pyb_sensors_type;
    for(uint16_t i = 0;i < SYS_SENSORS_MAX_CHANNEL_NUM;i++)
    {
        for(uint8_t j = 0;j < SYS_SENSORS_MAX_EVENT_NUM;j++)
        {
            sensor->event_is_enable[i][j] = false;
            sensor->event_callback[i][j] = mp_const_none;
            sensor->event_condition[i][j] = mp_const_none;
        }
    }
    sensor->sensor_name = sensor_name;
    sensor->sensor_number = sensor_number;
    sensor->sensor_obj_number = free_obj;
    
    MP_STATE_PORT(pyb_sensors_obj_all)[free_obj] = sensor;
    sensor->sensor_ptr = enable_external_sensor(sensor_name,sensor_number,MP_OBJ_FROM_PTR(sensor));
    if(sensor->sensor_ptr == R_FAIL)
    {
        m_del_obj(pyb_sensors_obj_t,sensor);
        MP_STATE_PORT(pyb_sensors_obj_all)[free_obj] = NULL;
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("sensros doesn't exist 3 %d %d %d"),free_obj,sensor_name,sensor_number);
    }
    sensor->sensor_channel_num = get_external_sensor_channel_num(sensor->sensor_ptr);
    return MP_OBJ_FROM_PTR(sensor);
}


static const mp_rom_map_elem_t pyb_sensors_locals_dict_table[] = {
    //传感器信息 
    //参数1:type
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_PTR(&sensors_info_obj) },
    //获取传感器数值
    //参数1:通道号  1~10
    { MP_ROM_QSTR(MP_QSTR_get_value), MP_ROM_PTR(&sensors_get_value_obj) },
    //设置传感器数值
    //参数1:列表[通道号  1~10,[功能,动作,参数...],通道号  1~10,[功能,动作,参数...],...]
    { MP_ROM_QSTR(MP_QSTR_set_value), MP_ROM_PTR(&sensors_set_value_obj) },
    //添加事件
    //参数1:通道号  1~10
    //参数2:事件条件(如果是基础事件)或者事件编号(如果是高级事件int类型)
    //参数3:事件回调函数
    { MP_ROM_QSTR(MP_QSTR_set_callback), MP_ROM_PTR(&sensors_set_callback_obj) },
    //删除事件
    //参数1:通道号  1~10
    //参数2:事件回调函数
    { MP_ROM_QSTR(MP_QSTR_del_callback), MP_ROM_PTR(&sensors_del_callback_obj) },
};

static MP_DEFINE_CONST_DICT(pyb_sensors_locals_dict, pyb_sensors_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    pyb_sensors_type,
    MP_QSTR_sensors,
    MP_TYPE_FLAG_NONE,
    make_new, pyb_sensors_make_new,
    locals_dict, &pyb_sensors_locals_dict
    );


MP_REGISTER_ROOT_POINTER(struct _pyb_sensors_obj_t *pyb_sensors_obj_all[SYS_SENSORS_NUM]);

