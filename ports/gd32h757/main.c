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


// Main entry point: initialise the runtime and execute demo strings.
int mp_main(int argc, char *argv[]) {
    printf("mp_main run argc=%d arg.1=%s\r\n", argc,argv[0]);
    printf("_estack=0x%lx stack_end=0x%lx stack_size=%ld\r\n", (uint32_t)&_estack, (uint32_t)&Heap_end,(uint32_t)((uint32_t)&_estack - (uint32_t)&Heap_end));
    printf("end=0x%lx _end=0x%lx Heap_end=0x%lx _estack=0x%lx\r\n", (uint32_t)&end, (uint32_t)&_end,(uint32_t)&Heap_end, (uint32_t)&_estack);
    // Stack limit init.
    mp_cstack_init_with_top(&_estack, (char *)&_estack - (char *)&Heap_end);

    static uint8_t heap[1024 * 128];
    // GC init
    gc_init(heap, &heap[1024 * 128]);

    mp_init();

    readline_init0();
    printf("pyexec_friendly_repl\r\n");
    for (;;) {
        if (pyexec_friendly_repl() != 0) {
            break;
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
