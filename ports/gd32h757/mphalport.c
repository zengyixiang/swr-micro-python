#include <string.h>

#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "extmod/misc.h"
#include "export_main.h"
#include <sys/time.h>
#include "py/ringbuf.h"
#include "debug.h"
#include "sys_timer.h"
#include "sys_sensors.h"
#include "sys_ui.h"
#include "RTOS/FreeRTOS/Source/include/FreeRTOS.h"
#include "RTOS/FreeRTOS/Source/include/task.h"
#include "RTOS/FreeRTOS/Source/include/timers.h"
#include "driver/timer/timer.h"

extern int uart_txc(char ch);
extern char uart_rxc(void);
static uint8_t stdin_ringbuf_array[260];
ringbuf_t stdin_ringbuf = {stdin_ringbuf_array, sizeof(stdin_ringbuf_array), 0, 0};



MP_WEAK uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) {

    return 0;
}
MP_WEAK int mp_hal_stdin_rx_chr(void) {
    for (;;) {
        int c = uart_rxc();//TODO: 这里要修改为ESP32那样的
        if(c != -1)
            return c;
        // if (MP_STATE_PORT(pyb_stdio_uart) != NULL && uart_rx_any(MP_STATE_PORT(pyb_stdio_uart))) {
        //     return uart_rx_char(MP_STATE_PORT(pyb_stdio_uart));
        // }
        // int dupterm_c = mp_os_dupterm_rx_chr();
        // if (dupterm_c >= 0) {
        //     return dupterm_c;
        // }
        // MICROPY_EVENT_POLL_HOOK
    }
}

MP_WEAK mp_uint_t mp_hal_stdout_tx_strn(const char *str, size_t len) {
    mp_uint_t ret = len;
    bool did_write = false;
#if 1
    for(int i = 0;i < len; i++){
        uart_txc(str[i]);
    }
    did_write = true;
#else
    if (MP_STATE_PORT(pyb_stdio_uart) != NULL) {
        uart_tx_strn(MP_STATE_PORT(pyb_stdio_uart), str, len);
        did_write = true;
    }
#endif
    int dupterm_res = mp_os_dupterm_tx_strn(str, len);
    if (dupterm_res >= 0) {
        did_write = true;
        ret = MIN((mp_uint_t)dupterm_res, ret);
    }
    return did_write ? ret : 0;
}

mp_uint_t mp_hal_ticks_cpu(void) {
    return 0;
}

mp_uint_t mp_hal_ticks_ms(void) {
    return get_sys_timer_us() * 1000;
}
mp_uint_t mp_hal_ticks_us(void) {
    return get_sys_timer_us();
}

uint64_t mp_hal_time_ns(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ns = tv.tv_sec * 1000000000ULL;
    ns += (uint64_t)tv.tv_usec * 1000ULL;
    return ns;
}

void mp_hal_delay_ms(mp_uint_t ms) {
    uint64_t us = (uint64_t)ms * 1000ULL;
    uint64_t dt;
    uint64_t t0 = get_sys_timer_us();
    for (;;) {
        mp_handle_pending(true);
        MP_THREAD_GIL_EXIT();
        uint64_t t1 = get_sys_timer_us();
        dt = t1 - t0;
        if (dt + portTICK_PERIOD_MS * 1000ULL >= us) {
            // doing a vTaskDelay would take us beyond requested delay time
            taskYIELD();
            MP_THREAD_GIL_ENTER();
            t1 = get_sys_timer_us();
            dt = t1 - t0;
            break;
        } else {
            ulTaskNotifyTake(pdFALSE, 1);
            MP_THREAD_GIL_ENTER();
        }
    }
    if (dt < us) {
        // do the remaining delay accurately
        mp_hal_delay_us(us - dt);
    }
}

void mp_hal_delay_us(mp_uint_t us) {
    // these constants are tested for a 240MHz clock
    const uint32_t this_overhead = 5;
    const uint32_t pend_overhead = 150;

    // return if requested delay is less than calling overhead
    if (us < this_overhead) {
        return;
    }
    us -= this_overhead;

    uint64_t t0 = get_sys_timer_us();
    for (;;) {
        uint64_t dt = get_sys_timer_us() - t0;
        if (dt >= us) {
            return;
        }
        if (dt + pend_overhead < us) {
            // we have enough time to service pending events
            // (don't use MICROPY_EVENT_POLL_HOOK because it also yields)
            mp_handle_pending(true);
        }
    }
}

extern TaskHandle_t  python_handle;
void mp_hal_wake_main_task(void) {
    xTaskNotifyGive(python_handle);
}

void mp_hal_wake_main_task_from_isr(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(python_handle, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}



// static mp_obj_t example_package___init__(void) {
//     if (!MP_STATE_VM(example_package_initialised)) {
//         // __init__ for builtins is called each time the module is imported,
//         //   so ensure that initialisation only happens once.
//         MP_STATE_VM(example_package_initialised) = true;
//         // mp_printf(&mp_plat_print, "example_package.__init__\n");
//     }
//     return mp_const_none;
// }
// static MP_DEFINE_CONST_FUN_OBJ_0(example_package___init___obj, example_package___init__);
// MP_REGISTER_ROOT_POINTER(int example_package_initialised);

extern void get_rtos_info(void);
static mp_obj_t RTOS_info(void) {
    get_rtos_info();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(RTOS_info_obj, RTOS_info);


static const mp_rom_map_elem_t pyb_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_Smart_Code_Firmware_Library) },
    // { MP_ROM_QSTR(MP_QSTR___init__), MP_ROM_PTR(&example_package___init___obj) },
    #if MICROPY_HW_ENABLE_SDCARD
    { MP_ROM_QSTR(MP_QSTR_SDCard), MP_ROM_PTR(&pyb_sdcard_type) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_delay), MP_ROM_PTR(&delay_module) },
    { MP_ROM_QSTR(MP_QSTR_timer), MP_ROM_PTR(&pyb_timer_type) },
    { MP_ROM_QSTR(MP_QSTR_sensors), MP_ROM_PTR(&pyb_sensors_type) },
    { MP_ROM_QSTR(MP_QSTR_ui), MP_ROM_PTR(&pyb_ui_type) },

    //  ?RTOS   ?   ?  
    { MP_ROM_QSTR(MP_QSTR_RTOS_info), MP_ROM_PTR(&RTOS_info_obj) },
};

static MP_DEFINE_CONST_DICT(pyb_module_globals, pyb_module_globals_table);

const mp_obj_module_t pyb_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&pyb_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_Smart_Code_Firmware_Library, pyb_module);
