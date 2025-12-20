#include "sys_ui.h"
#include "py/runtime.h"
#include <stdio.h>
#include <string.h>
#include "py/obj.h"
#include "export_main.h"
#include "lvgl/lvgl.h"
#include "gui_start.h"
#include "gui_python_run_page.h"
#include "gui_process.h"


extern void suspend_lvgl_task(void);
extern void  resume_lvgl_task(void);

#define LVGL_TAKE_MUTEX     suspend_lvgl_task()
#define LVGL_GIVE_MUTEX     resume_lvgl_task()


static lv_obj_t * screen;

// #define SYS_UI_NUM       1

// #define PYB_UI_OBJ_ALL_NUM MP_ARRAY_SIZE(MP_STATE_PORT(pyb_ui_obj_all))

#define EVENT_NUM       5

#define CALL_LVGL_FUNC(func, ...) \
    do { \
        LVGL_TAKE_MUTEX; \
        func(__VA_ARGS__); \
        LVGL_GIVE_MUTEX; \
    } while (0)

#define CALL_LVGL_FUNC_WITH_RESULT(func, result, ...) \
    do { \
        LVGL_TAKE_MUTEX; \
        result = func(__VA_ARGS__); \
        (void)result;   \
        LVGL_GIVE_MUTEX; \
    } while (0)

typedef enum _ui_widgets_type_t{
    WIDGETS_TYPE_NULL               = 0,
    WIDGETS_TYPE_SCREEN             = 1,		
    WIDGETS_TYPE_OBJ                = 2,		
    WIDGETS_TYPE_BUTTON             = 3,
    WIDGETS_TYPE_LABEL              = 4,
	WIDGETS_TYPE_IMG                = 5,
	WIDGETS_TYPE_RECTANGLE			= 6,
	WIDGETS_TYPE_CIRCLE				= 7,
} ui_widgets_type_t;

typedef enum _ui_attribute_type_t{
    ATTRIBUTE_TYPE_ALIGN            = 0,
    ATTRIBUTE_TYPE_SIZE             = 1,
    ATTRIBUTE_TYPE_POS              = 2,
    ATTRIBUTE_TYPE_TEXT     ,
	ATTRIBUTE_TYPE_IMG      ,
	ATTRIBUTE_TYPE_LAYER	,
	ATTRIBUTE_TYPE_ROTATION	,
	ATTRIBUTE_TYPE_BG_COLOR	,
	ATTRIBUTE_TYPE_RADIUS,
	ATTRIBUTE_TYPE_BORDER,
	ATTRIBUTE_TYPE_SHADOW,
	ATTRIBUTE_TYPE_TEXT_STYLE,
} ui_attribute_type_t;

typedef enum _ui_anim_type_t{
    ANIM_TYPE_MOVE                  = 0,
	ANIM_TYPE_ROTATION,
} ui_anim_type_t;


typedef enum _ui_flag_type_t{
	FLAG_TYPE_HIDDEN		= (1L << 0),
	FLAG_TYPE_ALL_HIDDEN    = (1L << 30),
}ui_flag_type_t;

typedef enum _ui_text_size_t{
	TEXT_SIZE_14 = 0,
	TEXT_SIZE_16,
	TEXT_SIZE_24,
	TEXT_SIZE_28,
	TEXT_SIZE_36,
	TEXT_SIZE_48,
}ui_text_size_t;

typedef struct{
	ui_text_size_t size;
	const lv_font_t *font;
}textSizeList_Typedef;

textSizeList_Typedef text_size_list[] = {
	{TEXT_SIZE_14, &lv_font_montserrat_14},
	{TEXT_SIZE_16, &lv_font_montserrat_16},
	{TEXT_SIZE_24, &lv_font_montserrat_24},
	{TEXT_SIZE_28, &lv_font_montserrat_28},
	{TEXT_SIZE_36, &lv_font_montserrat_36},
	{TEXT_SIZE_48, &lv_font_montserrat_48},
};
typedef struct _pyb_ui_obj_t {
    mp_obj_base_t base;
    ui_widgets_type_t type;

    lv_obj_t * obj;
    lv_anim_exec_xcb_t _anim_x_cb;
    //
    void * user_data[EVENT_NUM];
    mp_obj_t callback[EVENT_NUM];
    mp_int_t filter[EVENT_NUM];
    uint8_t event_is_enable[EVENT_NUM];
    uint8_t event_num;
} pyb_ui_obj_t;

void sys_ui_init0(void)
{
    // for (uint i = 0; i < PYB_UI_OBJ_ALL_NUM; i++) {
    //     MP_STATE_PORT(pyb_ui_obj_all)[i] = NULL;
    // }
}

static lv_obj_t * create_default_screen(lv_obj_t * parent)
{
    screen = lv_obj_create(gui_get_default_screen());
    lv_obj_align(screen,LV_ALIGN_CENTER,0,0);
    lv_obj_set_size(screen,200,200);
    lv_obj_set_style_bg_color(screen, lv_color_make(200, 200, 200), 0);
//    lv_disp_load_scr(screen);
    
    //TODO:

    return screen;
}

extern void interrupt_to_thread(void* callback,void* para);
static void sys_ui_do(lv_event_t * e)
{
    pyb_ui_obj_t * o = lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    if(o->event_num == 0) return;
    for(int i = 0;i < EVENT_NUM;i++)
    {
        if(o->filter[i] == code && o->event_is_enable[i] == true)
        {
            mp_obj_t callback = o->callback[i];
            if (callback != mp_const_none) {
                interrupt_to_thread(callback,MP_OBJ_FROM_PTR(o->user_data[i]));
            }
            break;
        }
    }
}

static mp_obj_t ui_create(mp_obj_t _type, mp_obj_t _parent) {
    ui_widgets_type_t type = mp_obj_get_int(_type);
    pyb_ui_obj_t *o = NULL;
    lv_obj_t *obj = NULL;
    lv_obj_t *parent_obj = NULL;

    // 判断 _parent 是否为 None
    if (_parent != mp_const_none) {
        pyb_ui_obj_t *parent = MP_OBJ_TO_PTR(_parent);
        parent_obj = parent->obj;
    }
    if (type == WIDGETS_TYPE_SCREEN) {
        // 屏幕类型创建  TODO:需要限制只有一个
        CALL_LVGL_FUNC_WITH_RESULT(create_default_screen, obj, parent_obj);
    } else if (type == WIDGETS_TYPE_OBJ) {
        // 普通对象创建
        CALL_LVGL_FUNC_WITH_RESULT(lv_obj_create, obj, parent_obj);
    } else if (type == WIDGETS_TYPE_BUTTON) {
        CALL_LVGL_FUNC_WITH_RESULT(lv_btn_create, obj, parent_obj);
    } else if (type == WIDGETS_TYPE_LABEL) {
        CALL_LVGL_FUNC_WITH_RESULT(lv_label_create, obj, parent_obj);
    }else if(type == WIDGETS_TYPE_IMG){
		CALL_LVGL_FUNC_WITH_RESULT(lv_img_create, obj, parent_obj);
	}
	else if((type == WIDGETS_TYPE_RECTANGLE) || (type == WIDGETS_TYPE_CIRCLE)){
		CALL_LVGL_FUNC_WITH_RESULT(lv_obj_create, obj, parent_obj);
	}
	else {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("ui_create_type(%d) doesn't exist"), type);
    }

    o = m_new_obj(pyb_ui_obj_t);
    memset(o, 0, sizeof(*o));
    o->base.type = &pyb_ui_type;
    o->type = type;
    o->obj = obj;

    CALL_LVGL_FUNC(lv_obj_add_event_cb,o->obj,sys_ui_do,LV_EVENT_ALL,(void*)o);
    return MP_OBJ_FROM_PTR(o);
}
static MP_DEFINE_CONST_FUN_OBJ_2(ui_create_obj, ui_create);


static mp_obj_t ui_set_attribute(size_t n_args, const mp_obj_t *args) {
    pyb_ui_obj_t * o = MP_OBJ_TO_PTR(args[0]);
    ui_attribute_type_t type = mp_obj_get_int(args[1]);
    lv_obj_t *obj = o->obj;
	int align , x, y, width, height;
	char img_path[128];
	const char * buff = NULL;
    switch(type)
	{
		case ATTRIBUTE_TYPE_ALIGN:
			align = mp_obj_get_int(args[2]);
			x = mp_obj_get_int(args[3]);
			y = mp_obj_get_int(args[4]);
			CALL_LVGL_FUNC(lv_obj_align,obj,align,x,y);
			break;
		case ATTRIBUTE_TYPE_SIZE:
			width = mp_obj_get_int(args[2]);
			height = mp_obj_get_int(args[3]);
			CALL_LVGL_FUNC(lv_obj_set_size,obj,width,height);
			break;
		case ATTRIBUTE_TYPE_POS:
			x = mp_obj_get_int(args[2]);
			y = mp_obj_get_int(args[3]);
			CALL_LVGL_FUNC(lv_obj_set_pos,obj,x,y);
			break;
		case ATTRIBUTE_TYPE_TEXT:
			buff = mp_obj_str_get_str(args[2]);
			CALL_LVGL_FUNC(lv_label_set_text_fmt,obj,buff);
			break;
		case ATTRIBUTE_TYPE_IMG:
			memset(img_path, 0, sizeof(img_path));
			buff = mp_obj_str_get_str(args[2]);
			sprintf(img_path, "%s/%s", gui_get_py_run_file_path(), buff);
			CALL_LVGL_FUNC(lv_img_set_src,obj,img_path);
			break;
		
		case ATTRIBUTE_TYPE_LAYER:
			x = mp_obj_get_int(args[2]);
			CALL_LVGL_FUNC(lv_obj_move_to_index,obj,x);
			break;
		
		case ATTRIBUTE_TYPE_ROTATION:
			x = mp_obj_get_int(args[2]);
			CALL_LVGL_FUNC(lv_obj_set_style_transform_rotation, obj, x * 10, LV_PART_MAIN | LV_STATE_DEFAULT);
			break;
		
		case ATTRIBUTE_TYPE_BG_COLOR:
			x = mp_obj_get_int(args[2]);
			CALL_LVGL_FUNC(lv_obj_set_style_bg_color, obj, lv_color_hex(x), LV_PART_MAIN | LV_STATE_DEFAULT);
			break;

		case ATTRIBUTE_TYPE_RADIUS:
			x = mp_obj_get_int(args[2]);
			CALL_LVGL_FUNC(lv_obj_set_style_radius, obj, x, LV_PART_MAIN | LV_STATE_DEFAULT);
			break;
		case ATTRIBUTE_TYPE_BORDER:
			x = mp_obj_get_int(args[2]);
			y = mp_obj_get_int(args[3]);
			CALL_LVGL_FUNC(lv_obj_set_style_border_color,obj,lv_color_hex(x), LV_PART_MAIN | LV_STATE_DEFAULT);
			CALL_LVGL_FUNC(lv_obj_set_style_border_width,obj,y, LV_PART_MAIN | LV_STATE_DEFAULT);
			break;
		case ATTRIBUTE_TYPE_SHADOW:
			
			break;
		case ATTRIBUTE_TYPE_TEXT_STYLE:
			x = mp_obj_get_int(args[2]);
			y = mp_obj_get_int(args[3]);
			width = sizeof(text_size_list) / sizeof(text_size_list[0]);
			for(height = 0; height < width; height++)
			{
				
				if(text_size_list[height].size == (ui_text_size_t)x)
				{
					CALL_LVGL_FUNC(lv_obj_set_style_text_font, obj, text_size_list[height].font, LV_PART_MAIN | LV_STATE_DEFAULT);
					break;
				}
			}
			if(height == width)
				CALL_LVGL_FUNC(lv_obj_set_style_text_font, obj, text_size_list[0].font, LV_PART_MAIN | LV_STATE_DEFAULT);
			CALL_LVGL_FUNC(lv_obj_set_style_text_color,obj,lv_color_hex(y), LV_PART_MAIN | LV_STATE_DEFAULT);
			break;
		default:
			mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("ui_set_attribute_type(%d) doesn't exist"), type);
			break;
	}
	return mp_const_none; 
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(ui_set_attribute_obj, 3, ui_set_attribute);


static mp_obj_t ui_get_attribute(size_t n_args, const mp_obj_t *args) {
    pyb_ui_obj_t * o = MP_OBJ_TO_PTR(args[0]);
    ui_attribute_type_t type = mp_obj_get_int(args[1]);
    lv_obj_t *obj = o->obj;
	lv_align_t algin;
	int val = 0, val1 = 0;
	mp_obj_t int1, int2;
	mp_obj_t items[2];
	lv_color_t color;
	lv_area_t coords;
    switch(type)
	{
		case ATTRIBUTE_TYPE_ALIGN:
			CALL_LVGL_FUNC_WITH_RESULT(lv_obj_get_style_align, algin, obj, LV_PART_MAIN);
			val = (int)algin;
			return mp_obj_new_int(val);
			break;
		case ATTRIBUTE_TYPE_SIZE:
			CALL_LVGL_FUNC_WITH_RESULT(lv_obj_get_width, val, obj);
			CALL_LVGL_FUNC_WITH_RESULT(lv_obj_get_height, val1, obj);
			int1 = mp_obj_new_int(val);
			int2 = mp_obj_new_int(val1);
			items[0] = int1;
			items[1] = int2;
			return mp_obj_new_list(2,items);
			break;
		case ATTRIBUTE_TYPE_POS:
			CALL_LVGL_FUNC(lv_obj_get_coords, obj, &coords);
			val = (coords.x1 + coords.x2) / 2;
			val1 = (coords.y1 + coords.y2) / 2;
			int1 = mp_obj_new_int(val);
			int2 = mp_obj_new_int(val1);
			items[0] = int1;
			items[1] = int2;
			return mp_obj_new_list(2,items);
			break;
		case ATTRIBUTE_TYPE_TEXT:
			break;
		case ATTRIBUTE_TYPE_IMG:
			break;
		
		case ATTRIBUTE_TYPE_LAYER:
			
			CALL_LVGL_FUNC_WITH_RESULT(lv_obj_get_index, val, obj);
			return mp_obj_new_int(val);
			break;
		
		case ATTRIBUTE_TYPE_ROTATION:
			CALL_LVGL_FUNC_WITH_RESULT(lv_obj_get_style_transform_rotation, val, obj, LV_PART_MAIN | LV_STATE_DEFAULT);
			val /= 10;
			return mp_obj_new_int(val);
			break;
		
		case ATTRIBUTE_TYPE_BG_COLOR:
			CALL_LVGL_FUNC_WITH_RESULT(lv_obj_get_style_bg_color, color, obj, LV_PART_MAIN | LV_STATE_DEFAULT);
			val = (color.red << 16) + (color.green << 8) + color.blue;
			return mp_obj_new_int(val);
			break;

		case ATTRIBUTE_TYPE_RADIUS:
			CALL_LVGL_FUNC_WITH_RESULT(lv_obj_get_style_radius, val, obj, LV_PART_MAIN | LV_STATE_DEFAULT);		
			return mp_obj_new_int(val);
			break;
		case ATTRIBUTE_TYPE_BORDER:
			CALL_LVGL_FUNC_WITH_RESULT(lv_obj_get_style_border_color, color, obj, LV_PART_MAIN | LV_STATE_DEFAULT);
			CALL_LVGL_FUNC_WITH_RESULT(lv_obj_get_style_border_width, val, obj, LV_PART_MAIN | LV_STATE_DEFAULT);
			val = (color.red << 16) + (color.green << 8) + color.blue;
			int1 = mp_obj_new_int(val);
			int2 = mp_obj_new_int(val1);

			items[0] = int1;
			items[1] = int2;
			return mp_obj_new_list(2,items);
			break;
		case ATTRIBUTE_TYPE_SHADOW:
			
			break;
		default:
			mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("ui_set_attribute_type(%d) doesn't exist"), type);
			break;
	}
	return mp_const_none; 
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(ui_get_attribute_obj, 2, ui_get_attribute);

static mp_obj_t ui_add_event_cb(size_t n_args, const mp_obj_t *args) {
    pyb_ui_obj_t * o = MP_OBJ_TO_PTR(args[0]);
    uint8_t i = 0;
    for(i = 0;i < EVENT_NUM;i++)
    {
        if(o->event_is_enable[i] == false) 
        {
            o->event_is_enable[i] = true;
            break;
        }
    }
    if(i >= EVENT_NUM)
    {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("ui_add_event_cb(%d) doesn't exist"), i);
    }
    o->event_num++;
    o->callback[i] = args[1];
    o->filter[i] = mp_obj_get_int(args[2]);
    o->user_data[i] = MP_OBJ_TO_PTR(args[3]);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(ui_add_event_cb_obj, 4, ui_add_event_cb);


static mp_obj_t ui_remove_event_cb(mp_obj_t _obj, mp_obj_t _event_cb) {
    pyb_ui_obj_t * o = MP_OBJ_TO_PTR(_obj);
    for(uint8_t i = 0;i < EVENT_NUM;i++)
    {
        if(o->event_is_enable[i] == true && o->callback[i] == _event_cb)
        {
            o->event_is_enable[i] = false;
            if(o->event_num > 0)
                o->event_num--;
        }
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(ui_remove_event_cb_obj,ui_remove_event_cb);


static void all_hidden_widget(lv_obj_t *obj, bool flag)
{
	LVGL_TAKE_MUTEX; 
	uint32_t cnt = lv_obj_get_child_cnt(obj);
	for(uint32_t index = 0; index < cnt; index ++)
	{
		if(flag == true)
			lv_obj_add_flag(lv_obj_get_child(obj, index), FLAG_TYPE_HIDDEN);
		else
			lv_obj_remove_flag(lv_obj_get_child(obj, index), FLAG_TYPE_HIDDEN);
	}
	LVGL_GIVE_MUTEX; 
}

static mp_obj_t ui_add_flag(mp_obj_t _obj, mp_obj_t _flag) {
    pyb_ui_obj_t * o = MP_OBJ_TO_PTR(_obj);
    int type = mp_obj_get_int(_flag);
    lv_obj_t *obj = o->obj;
	switch(type)
	{
		case FLAG_TYPE_HIDDEN:
			CALL_LVGL_FUNC(lv_obj_add_flag, obj, type);
			break;
		case FLAG_TYPE_ALL_HIDDEN:
			all_hidden_widget(obj, true);
			break;
		default:
			break;
	}
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(ui_add_flag_obj, ui_add_flag);



static mp_obj_t ui_clear_flag(mp_obj_t _obj, mp_obj_t _flag) {
    pyb_ui_obj_t * o = MP_OBJ_TO_PTR(_obj);
    int type = mp_obj_get_int(_flag);
    lv_obj_t *obj = o->obj;
	switch(type)
	{
		case FLAG_TYPE_HIDDEN:
			CALL_LVGL_FUNC(lv_obj_clear_flag, obj, type);
			break;
		case FLAG_TYPE_ALL_HIDDEN:
			all_hidden_widget(obj, false);
			break;
		default:
			break;
	}
    
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(ui_clear_flag_obj, ui_clear_flag);

static mp_obj_t ui_add_anim(size_t n_args, const mp_obj_t *args) {
    pyb_ui_obj_t * o = MP_OBJ_TO_PTR(args[0]);
    lv_obj_t *obj = o->obj;
    ui_anim_type_t type = mp_obj_get_int(args[1]);
	

    lv_anim_exec_xcb_t _anim_x_cb;
//	debug_printf("ui_add_anim type %d ANIM_TYPE_MOVE %d ANIM_TYPE_ROTATION %d\r\n", type, ANIM_TYPE_MOVE, ANIM_TYPE_ROTATION); 
    if(type == ANIM_TYPE_MOVE)
    {
		int angle = mp_obj_get_int(args[2]);
		int len = mp_obj_get_int(args[3]);
		int time_ms = mp_obj_get_int(args[4]);
//		debug_printf("angle %d len %d time_ms %d\r\n", angle, len, time_ms);
        CALL_LVGL_FUNC_WITH_RESULT(gui_create_move_anim, _anim_x_cb, obj, angle, len, time_ms);
		if(_anim_x_cb != NULL)
			o->_anim_x_cb = _anim_x_cb;
    }
	else if(type == ANIM_TYPE_ROTATION)
	{
		uint8_t mode = mp_obj_get_int(args[2]);
		int angle = mp_obj_get_int(args[3]);
		int time_ms = mp_obj_get_int(args[4]);
//		debug_printf("mode %d angle %d time_ms %d\r\n", mode, angle, time_ms);
        CALL_LVGL_FUNC_WITH_RESULT(gui_create_rotation_anim, _anim_x_cb, obj, mode, 0xffffff, angle * 10, time_ms);
		if(_anim_x_cb != NULL)
			o->_anim_x_cb = _anim_x_cb;
	}
    else
    {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("ui_add_anim_type(%d) doesn't exist"), type);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR(ui_add_anim_obj, 3, ui_add_anim);

static mp_obj_t ui_del_anim(mp_obj_t _obj) {
	pyb_ui_obj_t * o = MP_OBJ_TO_PTR(_obj);
	lv_obj_t *obj = o->obj;
	CALL_LVGL_FUNC(lv_anim_delete, obj, o->_anim_x_cb);
	o->_anim_x_cb = NULL;
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(ui_del_anim_obj, ui_del_anim);

static mp_obj_t ui_delete(mp_obj_t _obj) {
    pyb_ui_obj_t * o = MP_OBJ_TO_PTR(_obj);
    lv_obj_t *obj = o->obj;
    CALL_LVGL_FUNC(lv_obj_del,obj);
    m_del_obj(pyb_ui_obj_t,o);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(ui_delete_obj, ui_delete);

static const mp_rom_map_elem_t pyb_ui_locals_dict_table[] = {
    //创建对象
    { MP_ROM_QSTR(MP_QSTR_create), MP_ROM_PTR(&ui_create_obj) },
    //设置对象属性
    { MP_ROM_QSTR(MP_QSTR_set_attribute), MP_ROM_PTR(&ui_set_attribute_obj) },
    //获取对象属性
    { MP_ROM_QSTR(MP_QSTR_get_attribute), MP_ROM_PTR(&ui_get_attribute_obj) },
    //添加事件处理
    { MP_ROM_QSTR(MP_QSTR_add_event_cb), MP_ROM_PTR(&ui_add_event_cb_obj) },
    //删除事件处理
    { MP_ROM_QSTR(MP_QSTR_remove_event_cb), MP_ROM_PTR(&ui_remove_event_cb_obj) },
    //添加FLAG
    { MP_ROM_QSTR(MP_QSTR_add_flag), MP_ROM_PTR(&ui_add_flag_obj) },
    //清除FLAG
    { MP_ROM_QSTR(MP_QSTR_clear_flag), MP_ROM_PTR(&ui_clear_flag_obj) },
    //添加动画
    { MP_ROM_QSTR(MP_QSTR_add_anim), MP_ROM_PTR(&ui_add_anim_obj) },
    //删除动画
    { MP_ROM_QSTR(MP_QSTR_del_anim), MP_ROM_PTR(&ui_del_anim_obj) },
    //删除对象
    { MP_ROM_QSTR(MP_QSTR_delete), MP_ROM_PTR(&ui_delete_obj) },
};

static MP_DEFINE_CONST_DICT(pyb_ui_locals_dict, pyb_ui_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    pyb_ui_type,
    MP_QSTR_ui,
    MP_TYPE_FLAG_NONE,
    locals_dict, &pyb_ui_locals_dict
    );

// MP_REGISTER_ROOT_POINTER(struct _pyb_ui_obj_t *pyb_ui_obj_all[SYS_UI_NUM]);
