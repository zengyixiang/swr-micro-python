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

extern int uart_txc(char ch);
extern char uart_rxc(void);
extern void uart_xSemaphore_take(void);
extern void uart_xSemaphore_give(void);
static uint8_t stdin_ringbuf_array[260];
ringbuf_t stdin_ringbuf = {stdin_ringbuf_array, sizeof(stdin_ringbuf_array), 0, 0};



MP_WEAK uintptr_t mp_hal_stdio_poll(uintptr_t poll_flags) {

    return 0;
}
MP_WEAK int mp_hal_stdin_rx_chr(void) {
    for (;;) {
        return (int)uart_rxc();
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
    uart_xSemaphore_take();
    for(int i = 0;i < len; i++){
        uart_txc(str[i]);
    }
    uart_xSemaphore_give();
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

uint64_t mp_hal_time_ns(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t ns = tv.tv_sec * 1000000000ULL;
    ns += (uint64_t)tv.tv_usec * 1000ULL;
    return ns;
}



static mp_obj_t example_package___init__(void) {
    if (!MP_STATE_VM(example_package_initialised)) {
        // __init__ for builtins is called each time the module is imported,
        //   so ensure that initialisation only happens once.
        MP_STATE_VM(example_package_initialised) = true;
        // mp_printf(&mp_plat_print, "example_package.__init__\n");
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(example_package___init___obj, example_package___init__);
MP_REGISTER_ROOT_POINTER(int example_package_initialised);

extern void get_rtos_info(void);
static mp_obj_t RTOS_info(void) {
    get_rtos_info();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(RTOS_info_obj, RTOS_info);


static const mp_rom_map_elem_t pyb_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_Smart_Code_Firmware_Library) },
    { MP_ROM_QSTR(MP_QSTR___init__), MP_ROM_PTR(&example_package___init___obj) },
    #if MICROPY_HW_ENABLE_SDCARD
    { MP_ROM_QSTR(MP_QSTR_SDCard), MP_ROM_PTR(&pyb_sdcard_type) },
    #endif
    { MP_ROM_QSTR(MP_QSTR_delay), MP_ROM_PTR(&pyb_delay_type) },
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
