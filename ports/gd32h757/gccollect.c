#include "py/gc.h"
#include "py/mpthread.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/softtimer.h"
#include "gccollect.h"

void gc_collect(void) {
    // start the GC
    gc_collect_start();

    // trace the stack and registers
    gc_helper_collect_regs_and_stack();

    // trace root pointers from any threads
    #if MICROPY_PY_THREAD
    mp_thread_gc_others();
    #endif

    // trace soft timer nodes
    // soft_timer_gc_mark_all();

    // end the GC
    gc_collect_end();
    // gc_dump_info(&mp_plat_print);
}
