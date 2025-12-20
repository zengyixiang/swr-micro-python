/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2021 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <string.h>
#include "py/compile.h"
#include "py/runtime.h"
#include <stdio.h>
#include "py/mperrno.h"
#include "py/builtin.h"
#include "gccollect.h"
#include "py/gc.h"
#include "shared/runtime/pyexec.h"
#include "shared/readline/readline.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "sys_sensors.h"
#include "sys_timer.h"
#include "sys_ui.h"
#include "my_sdram.h"
#include "mp_main.h"
#include "app/sys.h"

// Main entry point: initialise the runtime and execute demo strings.
int mp_main(int argc, char *argv[]) {
    TaskStatus_t taskDetails;
    vTaskGetInfo(NULL, &taskDetails, pdTRUE, eInvalid);
    printf("pxStackBase=%lx pxTopOfStack=%lx stack_size=%ld stack_size_4=%ld uxBasePriority=%ld\r\n", (uint32_t)taskDetails.pxStackBase, (uint32_t)taskDetails.pxTopOfStack, (uint32_t)taskDetails.pxTopOfStack - (uint32_t)taskDetails.pxStackBase, ((uint32_t)taskDetails.pxTopOfStack - (uint32_t)taskDetails.pxStackBase) / sizeof(uintptr_t), taskDetails.uxBasePriority);
    
    mp_thread_init(taskDetails.pxStackBase,((uint32_t)taskDetails.pxTopOfStack - (uint32_t)taskDetails.pxStackBase) / sizeof(uintptr_t), taskDetails.uxBasePriority);

    mp_cstack_init_with_top((void*)taskDetails.pxTopOfStack, (uint32_t)taskDetails.pxTopOfStack - (uint32_t)taskDetails.pxStackBase);

    gc_init((void*)SDRAM_DEVICE0_ADDR, (void*)(SDRAM_DEVICE0_ADDR + 1024 * 1024));

    mp_init();
    timer_init0();
    sys_sensors_init();
    sys_ui_init0();
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_System_slash_SysLib));
    init_sdcard_fs();
    readline_init0();

    while(1)
    {
        if(get_py_state() == PY_STATE_RUN_FILE)
        {
            pyexec_file_if_exists(PY_FILE_NAME);
            mp_thread_deinit();
            py_sys_exit();
			free_py_mem();
            vTaskDelay(10);
            gc_collect();
            change_py_state(PY_STATE_IDLE);
        }
        else if(get_py_state() == PY_STATE_REPL)
        {
            pyexec_friendly_repl();
            mp_thread_deinit();
            py_sys_exit();
			free_py_mem();
            vTaskDelay(10);
            gc_collect();
            change_py_state(PY_STATE_IDLE);  
        }
        else
        {
			fwdgt_counter_reload();
            change_py_state(PY_STATE_IDLE);
            vTaskDelay(100);
        }
    }
    mp_deinit();
    return 0;
}

// Called if an exception is raised outside all C exception-catching handlers.
void nlr_jump_fail(void *val) {
    for (;;) {
    }
}

// mp_lexer_t *mp_lexer_new_from_file(qstr filename) {
//     mp_raise_OSError(MP_ENOENT);
// }
// mp_import_stat_t mp_import_stat(const char *path) {
//     return MP_IMPORT_STAT_NO_EXIST;
// }

mp_uint_t mp_hal_ticks_ms(void) {
    return 0;
}
mp_uint_t mp_hal_ticks_us(void) {
    return 0;
}
mp_uint_t mp_hal_ticks_cpu(void) {
    return 0;
}
void mp_hal_delay_ms(mp_uint_t Delay){

}
void mp_hal_delay_us(mp_uint_t Delay){
    
}
#ifndef NDEBUG
// Used when debugging is enabled.
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    for (;;) {
    }
}
#endif
